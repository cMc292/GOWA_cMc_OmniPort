-- Esquema de referencia de la bitácora de GOWA_cMc_OmniPort.
-- La fuente principal es docs/bitacora-codigo.csv.
-- Uso con SQLite:
--   sqlite3 bitacora-codigo.sqlite < docs/bitacora-codigo.schema.sql
--   .mode csv
--   .import docs/bitacora-codigo.csv procesos

CREATE TABLE IF NOT EXISTS procesos (
    id          TEXT PRIMARY KEY,
    fecha       TEXT NOT NULL,
    modulo      TEXT NOT NULL,
    proceso     TEXT NOT NULL,
    archivos    TEXT,
    verificacion TEXT,
    estado      TEXT NOT NULL CHECK (estado IN (
        'base', 'implementado', 'verificado', 'corregido',
        'sustituido', 'pendiente', 'no-publicar'
    )),
    commit_hash TEXT
);

CREATE TABLE IF NOT EXISTS fallos (
    id          TEXT PRIMARY KEY,
    sintoma     TEXT NOT NULL,
    causa       TEXT NOT NULL,
    correccion  TEXT NOT NULL,
    estado      TEXT NOT NULL
);

CREATE TABLE IF NOT EXISTS binarios (
    nombre      TEXT PRIMARY KEY,
    sha256      TEXT NOT NULL,
    estado      TEXT NOT NULL,
    nota        TEXT
);
