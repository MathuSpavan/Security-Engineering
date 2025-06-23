#!/usr/bin/env bash

# === Einstellungen ===
words_file="words.txt"
hash_file="user_hashes.txt"
hash_dir="hashes"
verbose=false

# === Argumente prüfen ===
if [ "$1" = "-v" ]; then
  verbose=true
  echo "[*] Verbose-Modus aktiviert"
fi

# === Hash-Verzeichnis vorbereiten ===
mkdir -p "$hash_dir"

echo "[*] Phase 1: Erzeuge Hash-Tabellen…"
while read -r name hash; do
  salt=$(echo "$hash" | cut -d'$' -f3)
  outfile="$hash_dir/${salt}.txt"

  if [ ! -f "$outfile" ]; then
    $verbose && echo "  [+] Generiere Hashes für $name (Salt: $salt)"
    openssl passwd -1 -salt "$salt" -in "$words_file" |
      paste "$words_file" - > "$outfile"
  else
    $verbose && echo "  [-] Überspringe $name – Hash-Datei existiert"
  fi
done < "$hash_file"

echo ""
echo "[*] Phase 2: Suche nach passenden Passwörtern…"
echo ""

while read -r name hash; do
  salt=$(echo "$hash" | cut -d'$' -f3)
  hashfile="$hash_dir/${salt}.txt"

  $verbose && echo "  [~] Suche Passwort für $name (Salt: $salt)"

  result=$(awk -v target="$hash" '$2 == target { print $1; exit }' "$hashfile")

  if [ -n "$result" ]; then
    echo "$name: $result"
  else
    echo "$name: NICHT GEFUNDEN"
  fi
done < "$hash_file"

