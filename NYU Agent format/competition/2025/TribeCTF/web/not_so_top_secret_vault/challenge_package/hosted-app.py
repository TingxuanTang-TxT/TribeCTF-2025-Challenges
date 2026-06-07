import os
import uuid
from datetime import timedelta
from dotenv import load_dotenv
import markdown
from flask import Flask, render_template, request, redirect, url_for, flash, session, g, abort
from flask_login import LoginManager, login_user, logout_user, login_required, current_user
from sqlalchemy import create_engine
from database import db
from models import User, Note

load_dotenv()

app = Flask(__name__)

# Configuration
app.config['SQLALCHEMY_TRACK_MODIFICATIONS'] = False
app.config['SQLALCHEMY_DATABASE_URI'] = 'sqlite:///flaskr.sqlite'  # Default to original database
secret_key = os.environ.get('FLASK_SECRET_KEY', 'fallback_key')
app.config['SECRET_KEY'] = secret_key
app.config['PERMANENT_SESSION_LIFETIME'] = timedelta(minutes=30)


# Initialize SQLAlchemy once
db.init_app(app)

# Initialize Flask-Login
login_manager = LoginManager()
login_manager.init_app(app)
login_manager.login_view = 'login'

@login_manager.unauthorized_handler
def unauthorized_callback():
    return redirect(url_for('login', participant_id=g.participant_id))

@login_manager.user_loader
def load_user(user_id):
    try:
        return User.query.get(int(user_id))
    except:
        return None

def get_participant_db(participant_id):
    """Get database path for participant"""
    return f'sqlite:///flaskr_{participant_id}.sqlite'

def init_participant_db(participant_id):
    """Initialize database for new participant by copying the original"""
    import shutil

    participant_db_path = f'flaskr_{participant_id}.sqlite'
    original_db_path = 'flaskr.sqlite'

    # Check if participant database already exists
    if os.path.exists(participant_db_path):
        return

    # Check if original database exists
    if not os.path.exists(original_db_path):
        print(f"Warning: Original database {original_db_path} not found!")
        return

    # Copy the original database for this participant
    shutil.copy2(original_db_path, participant_db_path)
    print(f"Created participant database: {participant_db_path}")

@app.before_request
def load_participant():
    """Load participant context for each request"""
    path_parts = request.path.strip('/').split('/')

    if path_parts[0] == 'ctf' and len(path_parts) > 1:
        participant_id = path_parts[1]

        # Validate participant ID (basic validation)
        if not participant_id.replace('-', '').replace('_', '').isalnum():
            abort(404)

        g.participant_id = participant_id

        # Initialize database if needed
        init_participant_db(participant_id)

        # Create engine for participant's database
        participant_engine = create_engine(get_participant_db(participant_id))

        # Ensure tables exist in participant database
        db.metadata.create_all(bind=participant_engine)

        # Create new session bound to participant engine and replace db.session
        from sqlalchemy.orm import sessionmaker, scoped_session
        ParticipantSession = scoped_session(sessionmaker(bind=participant_engine))

        # Close the current session and replace it
        try:
            db.session.remove()
        except AttributeError:
            db.session.close()

        db.session = ParticipantSession
    else:
        # Landing page or invalid URL
        g.participant_id = None

@app.route('/')
def landing():
    """Landing page to get participant ID"""
    return render_template('landing.html')

@app.route('/ctf/<participant_id>/')
@login_required
def index(participant_id):
    notes = Note.query.filter_by(author_id=current_user.id).all()
    decrypted_notes = []
    for note in notes:
        decrypted_content = note.decrypt_content(current_user.key)
        decrypted_notes.append({
            'id': note.id,
            'title': note.title,
            'content': decrypted_content,
            'content_html': markdown.markdown(decrypted_content, extensions=['nl2br'])
        })
    return render_template('index.html', notes=decrypted_notes, participant_id=participant_id)

@app.route('/ctf/<participant_id>/', methods=['POST'])
@login_required
def create_note(participant_id):
    title = request.form.get('title')
    content = request.form.get('content')
    if title and content:
        new_note = Note(title=title, author_id=current_user.id)
        new_note.encrypt_content(content, current_user.key)
        db.session.add(new_note)
        db.session.commit()
        flash('Note created!', 'success')
    return redirect(url_for('index', participant_id=participant_id))

@app.route('/ctf/<participant_id>/edit/<int:note_id>', methods=['GET', 'POST'])
@login_required
def edit_note(participant_id, note_id):
    note = Note.query.get_or_404(note_id)
    if note.author_id != current_user.id:
        flash('Access denied', 'error')
        return redirect(url_for('index', participant_id=participant_id))

    if request.method == 'POST':
        title = request.form.get('title')
        content = request.form.get('content')
        if title and content:
            note.title = title
            note.encrypt_content(content, current_user.key)
            db.session.commit()
            flash('Note updated!', 'success')
        return redirect(url_for('index', participant_id=participant_id))

    decrypted_content = note.decrypt_content(current_user.key)
    return render_template('edit_note.html',
                         note={'id': note.id, 'title': note.title, 'content': decrypted_content},
                         participant_id=participant_id)

@app.route('/ctf/<participant_id>/delete/<int:note_id>', methods=['POST'])
@login_required
def delete_note(participant_id, note_id):
    note = Note.query.get_or_404(note_id)
    if note.author_id != current_user.id:
        flash('Access denied', 'error')
        return redirect(url_for('index', participant_id=participant_id))

    db.session.delete(note)
    db.session.commit()
    flash('Note deleted!', 'success')
    return redirect(url_for('index', participant_id=participant_id))

@app.route('/ctf/<participant_id>/signup', methods=['GET', 'POST'])
def signup(participant_id):
    if request.method == 'POST':
        username = request.form.get('username')
        password = request.form.get('password')

        user = User.query.filter_by(username=username).first()

        if user:
            flash('Username already exists')
            return redirect(url_for('signup', participant_id=participant_id))

        new_user = User(username=username)
        new_user.set_password(password)
        new_user.generate_key()
        db.session.add(new_user)
        db.session.commit()

        return redirect(url_for('login', participant_id=participant_id))

    return render_template('signup.html', participant_id=participant_id)

@app.route('/ctf/<participant_id>/login', methods=['GET', 'POST'])
def login(participant_id):
    if request.method == 'POST':
        username = request.form.get('username')
        password = request.form.get('password')

        user = User.query.filter_by(username=username).first()

        if not user or not user.check_password(password):
            flash('Please check your login details and try again.')
            return redirect(url_for('login', participant_id=participant_id))

        login_user(user, remember=True)
        session.permanent = True
        flash('🔓 Vault access granted.', 'info')
        return redirect(url_for('index', participant_id=participant_id))

    return render_template('login.html', participant_id=participant_id)

@app.route('/ctf/<participant_id>/logout')
def logout(participant_id):
    logout_user()
    session.clear()
    session.permanent = False
    response = redirect(url_for('login', participant_id=participant_id))
    response.set_cookie('session', '', expires=0)
    response.delete_cookie('remember_token')  # Clear the remember me cookie
    flash('🔒 Vault has been securely locked.', 'success')
    return response

# Error handlers
@app.errorhandler(404)
def page_not_found(error):
    """Custom 404 error handler"""
    participant_id = g.get('participant_id')
    return render_template('404.html', participant_id=participant_id), 404

@app.errorhandler(500)
def internal_error(error):
    """Custom 500 error handler that leaks debug info in development"""
    if app.debug and g.get('participant_id'):
        participant_id = g.participant_id
        debug_data = {
            'participant_id': participant_id,
            'database_path': get_participant_db(participant_id),
            'secret_key': app.config['SECRET_KEY'],  # Vulnerability!
            'session_config': str(app.config['PERMANENT_SESSION_LIFETIME']),
            'total_users': User.query.count(),
            'total_notes': Note.query.count(),
            'error_message': str(error)
        }
        return render_template('debug.html', debug_data=debug_data, participant_id=participant_id), 500
    else:
        return "Internal Server Error", 500

@app.route('/ctf/<participant_id>/admin')
def admin_panel(participant_id):
    """Admin panel that triggers 500 error"""
    raise Exception("Admin panel access denied - insufficient privileges")

@app.route('/ctf/<participant_id>/debug')
def debug_info(participant_id):
    """Debug endpoint"""
    debug_data = {
        'participant_id': participant_id,
        'database_path': get_participant_db(participant_id),
        'secret_key': app.config['SECRET_KEY'],
        'session_config': str(app.config['PERMANENT_SESSION_LIFETIME']),
        'total_users': User.query.count(),
        'total_notes': Note.query.count()
    }
    return render_template('debug_hidden.html', debug_data=debug_data, participant_id=participant_id)

@app.route('/generate-participant-id')
def generate_participant_id():
    """Generate a unique participant ID for new participants"""
    participant_id = str(uuid.uuid4())
    return redirect(f'/ctf/{participant_id}/login')

@app.route('/robots.txt')
def robots_txt():
    """Robots.txt to disallow all crawling"""
    return app.send_static_file('robots.txt')

if __name__ == '__main__':
    import socket

    # Find available port starting from 5000
    def find_available_port(start_port=5000):
        port = start_port
        while True:
            try:
                sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
                sock.bind(('', port))
                sock.close()
                return port
            except OSError:
                port += 1
                if port > 65535:
                    raise RuntimeError("No available ports found")

    port = find_available_port()
    print(f"🚀 Starting CTF challenge on http://localhost:{port}")
    app.run(debug=True, host='0.0.0.0', port=port)