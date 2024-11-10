import pathlib
import argparse
import sqlite3
import os

data_dir = pathlib.Path(__file__).parent.parent / "data" / "changelog"

def compare_versions(version1: str, version2: str) -> int:
    # Split the version strings into parts
    v1_parts = [int(part) for part in version1.split('.')]
    v2_parts = [int(part) for part in version2.split('.')]

    # Compare each part
    for v1, v2 in zip(v1_parts, v2_parts):
        if v1 > v2: return 1
        elif v1 < v2: return -1

    # If one version has more parts (e.g., 1.2 vs 1.2.1), compare remaining parts
    if len(v1_parts) > len(v2_parts): return 1
    elif len(v1_parts) < len(v2_parts): return -1
    else: return 0
def version_key(version: str):
    # Convert the version string into a tuple of integers
    return tuple(int(part) for part in version.split('.'))

def to_sqlite_helper(conn):
    create_ver_table ="""
CREATE TABLE "versions" (
        "id"	INTEGER UNIQUE, 
        "major"	INTEGER NOT NULL,
        "minor"	INTEGER NOT NULL,
        "patch"	INTEGER NOT NULL,
        "ref"	TEXT,
        "date"	TEXT,
        "blurb"	TEXT,
        "version" TEXT GENERATED ALWAYS AS (major || '.' || minor || '.' || patch) STORED,
        PRIMARY KEY("id" AUTOINCREMENT),
        UNIQUE(major, minor, patch)
);
"""

    create_types_table="""
CREATE TABLE "types" (
        "name"	TEXT UNIQUE COLLATE NOCASE,
        PRIMARY KEY("name")
);
"""

    create_changes_table="""
CREATE TABLE "changes" (
        "version"	INTEGER,
        "ref"           INTEGER,
        "type"	INTEGER NOT NULL,
        "priority"	INTEGER,
        "message"	TEXT,
        FOREIGN KEY("type") REFERENCES "types"("rowid"),
        FOREIGN KEY("version") REFERENCES "versions"("id")
);
"""
    import csv
    cursor = conn.cursor()

    # Create table with info about versions
    cursor.execute("DROP TABLE IF EXISTS versions;")
    cursor.execute(create_ver_table)
    with open(data_dir/"versions.csv", "r") as f:
        reader = csv.reader(f)
        next(reader)
        for vernum,ref,date,blurb in reader: # Skip first line, which contains headers
            major,minor,patch=vernum.split(".")
            cursor.execute("INSERT INTO versions(major, minor, patch, ref, date, blurb) VALUES (?,?,?,?,?,?);", (major, minor, patch, ref, date, blurb))
    conn.commit()

    # Create table with info about change types
    cursor.execute("DROP TABLE IF EXISTS types;")
    cursor.execute(create_types_table)
    for name in ["Added", "Changed", "Fixed", "Optimization", "Security", "Deprecated", "Removed"]:
        cursor.execute("INSERT INTO types(name) VALUES(?)", (name,))
    conn.commit()

    # Create table with info about changes
    cursor.execute("DROP TABLE IF EXISTS changes;")
    cursor.execute(create_changes_table)
    with open(data_dir/"changes.csv") as f:
        reader = csv.reader(f)
        next(reader) # Skip first line, which contains headers
        for type, priority, ver, ref, msg in reader:
            # Convert version string into row reference
            cursor.execute("SELECT id FROM versions WHERE version = ?", (ver,))
            if (ver_row := cursor.fetchone()): ver_id = ver_row[0]
            else: ver_id = None

            # Convert change type into row reference
            cursor.execute("SELECT rowid FROM types WHERE name = ?", (type,))
            if (type_row := cursor.fetchone()): type_id = type_row[0]
            else: type_id = None

            # Convert string priority to int
            priority_int = 0
            if priority.lower() == "major": priority_int = 2
            elif priority.lower() == "minor": priority_int = 1

            cursor.execute("INSERT INTO changes(version, type, priority, message, ref) VALUES(?,?,?,?,?)",
                           (ver_id, type_id, priority_int, msg,ref))
    conn.commit()

def to_sqlite(args):
    try: os.remove(args.db)
    except OSError: pass
    f = pathlib.Path(args.db)
    conn = sqlite3.connect(f)
    to_sqlite_helper(conn)
    conn.close()


def stringize_version(conn, version):
    # Print header
    date,version_id = conn.execute("SELECT date, id FROM versions WHERE version = ?", (version,)).fetchone()[0:2]
    hdr_date = f"Released {date}" if date!="??" else "Unreleased"
    hdr_title = f"# Version [{version}](https://github.com/Matthew-McRaven/Pepp/releases/v{version})"
    hdr_blurb = conn.execute("SELECT blurb FROM versions WHERE version = ?", (version,)).fetchone()[0]

    # Print (sorted) sections
    sections = []
    section_order = [x for x in conn.execute("SELECT rowid, name FROM types ORDER BY rowid;").fetchall()]
    for section_id, section_name in section_order:
        changes = conn.execute("SELECT ref, message FROM changes WHERE version = ? AND type = ? ORDER BY priority DESC", (version_id, section_id)).fetchall()
        if len(changes) == 0: continue
        section = f"## {section_name}\n\n"
        change_strs = []
        for ref, message in changes:
            if not ref: change_strs.append(f" - {message}")
            else: change_strs.append(f" - {message}. See [#{ref}](https://github.com/Matthew-McRaven/Pepp/issues/{ref})")
        sections.append(section + "\n".join(change_strs)+"\n")
    n="\n"
    return f"""{hdr_title} -- {hdr_date}\n\n{hdr_blurb}\n\n{n.join(sections)}"""

def to_text(args):
    conn = sqlite3.connect(":memory:")
    to_sqlite_helper(conn)
    cursor = conn.cursor()
    # Get version range
    cursor.execute("SELECT version FROM versions ORDER BY id;")
    all_versions = sorted([row[0] for row in cursor.fetchall()], key=version_key)
    begin = args.begin if args.begin else all_versions[0]
    assert begin in all_versions, f"Version {begin} not found in database"
    end = args.end if args.end else all_versions[-1]
    assert end in all_versions, f"Version {end} not found in database"
    to_list = reversed(all_versions[all_versions.index(begin):all_versions.index(end)+1])
    items = [stringize_version(conn, v) for v in to_list]
    output = "\n\n".join(items).rstrip()
    with open(args.changelog, "w") as f: f.write(output+"\n")
    conn.close()


def normalize(args): raise NotImplemented()


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Command Line Tool")
    subparsers = parser.add_subparsers(dest='command', help='Available subcommands')

    # Subcommand: to_sql
    parser_to_sql = subparsers.add_parser('to_sql', help='Convert to SQL')
    parser_to_sql.add_argument('db', help='Path to output SQLite DB')
    parser_to_sql.set_defaults(func=to_sqlite)

    # Subcommand: to_text
    parser_to_text = subparsers.add_parser('to_text', help='Create a MD changelog for a range of versions')
    parser_to_text.add_argument('changelog', help='File into which to write changelog')
    parser_to_text.add_argument('--begin', dest='begin', help='First version to include in changelog', default=None)
    parser_to_text.add_argument('--end', dest='end', help='Last version to include in changelog (inclusive)', default=None)
    parser_to_text.set_defaults(func=to_text)

    # Subcommand: null_to_latest
    parser_normalize = subparsers.add_parser('null_to_latest', help='Convert NULL versions to use latest version')
    parser_normalize.set_defaults(func=normalize)

    args = parser.parse_args()
    if hasattr(args, 'func'): args.func(args)
    else: parser.print_help()
