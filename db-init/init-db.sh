#!/usr/bin/env bash
set -euo pipefail

MYSQL_HOST="${MYSQL_HOST:-mysql}"
MYSQL_PORT="${MYSQL_PORT:-3306}"
MYSQL_ROOT_USER="${MYSQL_ROOT_USER:-root}"
MYSQL_ROOT_PASSWORD="${MYSQL_ROOT_PASSWORD:?MYSQL_ROOT_PASSWORD is not set}"

SQL_DIR="/app"
LOCK_DIR="/etc/db-init"
LOCK_FILE="${LOCK_DIR}/.db_initialized.lock"

mkdir -p "$LOCK_DIR"

if [ -f "$LOCK_FILE" ]; then
  echo "[db-init] Database is already initialized (lock file exists: ${LOCK_FILE}). Skipping."
  exit 0
fi

SQL_STEPS=(
  "sql/create/create_mysql.sql:"
  "sql/base/auth_database.sql:auth"
  "sql/base/characters_database.sql:characters"
  "tdb/TDB_world_735.26972_2025_05_11.sql:world"
  "tdb/TDB_hotfixes_735.26972_2025_05_11.sql:hotfixes"
)

mysql_root() {
  mysql -h "$MYSQL_HOST" -P "$MYSQL_PORT" -u "$MYSQL_ROOT_USER" -p"$MYSQL_ROOT_PASSWORD" "$@"
}

echo "[db-init] Waiting for MySQL availability at ${MYSQL_HOST}:${MYSQL_PORT}..."
until mysql_root -e "SELECT 1" >/dev/null 2>&1; do
  sleep 2
done
echo "[db-init] MySQL is available."

for step in "${SQL_STEPS[@]}"; do
  rel_path="${step%%:*}"
  target_db="${step#*:}"
  file="${SQL_DIR}/${rel_path}"

  if [ ! -f "$file" ]; then
    echo "[db-init] ERROR: file ${file} not found." >&2
    exit 1
  fi

  if [ -n "$target_db" ]; then
    echo "[db-init] Executing ${rel_path} -> database '${target_db}'..."
    mysql_root "$target_db" < "$file"
  else
    echo "[db-init] Executing ${rel_path}..."
    mysql_root < "$file"
  fi
done

touch "$LOCK_FILE"
echo "[db-init] Done. All SQL scripts executed and lock file created."