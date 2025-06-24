#!/bin/bash
# Skript: bank_transaction.sh
# ---------------------------------------
# Simuliert den Server für ein Online-Banking-System mit TAN-Prüfung.
# Dieses Skript erwartet im Verzeichnis TAN pro Benutzername eine Datei <Benutzernamename>.txt mit der aktuellen TAN-Liste.

# 1) Unterbinde Abbruch mit STRG+C
trap "echo 'Schließen deaktiviert';" SIGINT #trap fängt Signale ab und SIGINT ist das Signal für STRG+C

#https://openbook.rheinwerk-verlag.de/shell_programmierung/shell_009_002.htm

# 2) Endlosschleife
while true; do
  echo -n "Benutzername: " # -n ist nur für die Optik damit die Eingabe in der gleichen Zeile erfolgt
  read Benutzername

  # Programm beenden, wenn Benutzer 'ende' eingibt
  if [[ "$Benutzername" == "ende" ]]; then
    echo "Programm beendet."
    exit 0
  fi

  echo -n "TAN: " 
  read tan_input

  TAN_FILE="TAN/${Benutzername}.txt"
  # Prüfen, ob Datei existiert
  if [ ! -f "$TAN_FILE" ]; then     #-f exestiert die Datei und ist eine regular Datei
    echo "Zugriff verweigert: Benutzer nicht gefunden."
    continue # continue überspringt den Rest der Schleife und springt zum Anfang zurück
  fi

  # Lese erste TAN aus der Datei (nach der Kommentarzeile)
  current_tan=$(sed -n '2p' "$TAN_FILE") #Stream Editor (sed) liest die zweite Zeile der Datei, die die aktuelle TAN enthält
                                         # n unterdrückt die Ausgabe, p für print gibt die Zeile aus
                                         #
  
  # Prüfen, ob Liste erschöpft ist -> Datei löschen und Programm beenden
  if [ -z "$current_tan" ]; then                            # z ist string zero, also leer
    echo " TAN-Liste erschöpft."
    echo "Lösche TAN-Datei für Benutzer '$Benutzername'."
    rm -f "$TAN_FILE"
    echo "Programm beendet."
    exit 0
  fi

  # TAN validieren
  if [ "$tan_input" = "$current_tan" ]; then
    echo "Zugriff erlaubt"
    # Verbrauchten TAN aus der Liste entfernen (zweite Zeile)
    tmpfile=$(mktemp)
    awk 'NR!=2' "$TAN_FILE" > "$tmpfile" && mv "$tmpfile" "$TAN_FILE"
  else
    echo "Zugriff verweigert: falsche TAN."
  fi

done
