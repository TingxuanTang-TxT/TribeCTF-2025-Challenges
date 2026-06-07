# 🔐 Not So Secret Vault - CTF Challenge

A Flask session forgery challenge with participant isolation.

## 🚀 Quick Start

```bash
cd challenge_package
./deploy.sh production
```

Visit the URL shown in output to start.

## 🎯 Challenge

**Objective**: Access admin's secret notes containing the flag.

**Approach**: Find and exploit session vulnerabilities to impersonate the admin user.

## 🔧 Management

```bash
./cleanup.sh     # Stop and cleanup
docker-compose logs -f    # View logs
```

## 💡 Notes

- Each participant gets isolated environment
- Admin credentials (only for us, organizers): `ops.manager/i@I1£H731W/+`
- Flag: `TribeCTF{s3ss10n_f0rg3ry_1s_d4ng3r0us}`
- Difficulty: Medium

---
