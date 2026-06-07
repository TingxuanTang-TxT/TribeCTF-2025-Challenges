from database import db
from werkzeug.security import generate_password_hash, check_password_hash
from flask_login import UserMixin
from cryptography.fernet import Fernet

class User(UserMixin, db.Model):
    id = db.Column(db.Integer, primary_key=True)
    username = db.Column(db.String(80), unique=True, nullable=False)
    password = db.Column(db.String(200), nullable=False)
    key = db.Column(db.LargeBinary, nullable=False)

    def set_password(self, password):
        self.password = generate_password_hash(password)

    def check_password(self, password):
        return check_password_hash(self.password, password)
    
    def generate_key(self):
        self.key = Fernet.generate_key()

class Note(db.Model):
    id = db.Column(db.Integer, primary_key=True)
    title = db.Column(db.String(100), nullable=False)
    content = db.Column(db.LargeBinary, nullable=False)
    author_id = db.Column(db.Integer, db.ForeignKey('user.id'), nullable=False)

    author = db.relationship('User', backref=db.backref('notes', lazy=True))

    def encrypt_content(self, content, key):
        f = Fernet(key)
        self.content = f.encrypt(content.encode('utf-8'))

    def decrypt_content(self, key):
        f = Fernet(key)
        return f.decrypt(self.content).decode('utf-8')
