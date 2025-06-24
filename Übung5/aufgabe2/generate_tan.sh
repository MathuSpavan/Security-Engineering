#!/bin/bash
# Skript: generate_tan.sh
# ---------------------------------------
# Verwendung: ./generate_tan.sh <username> <Anzahl_TANs>
# Dieses Skript generiert mit dem Lamport-Verfahren eine Liste von n Einmalpasswörtern (TANs).
# Hier mit SHA-256 und optional pro-User Salt.

#---- Überprüfe Parameteranzahl ----
if [ "$#" -ne 2 ]; then
  echo "Usage: $0 <username> <number_of_tans>"
  exit 1
fi

USERNAME="$1"
NUM_TANS="$2"

#---- Validierung: Anzahl_TANs muss positive Ganzzahl sein ----
if ! [[ "$NUM_TANS" =~ ^[1-9][0-9]*$ ]]; then
  echo "Fehler: Anzahl_TANs muss eine positive Ganzzahl sein."
  exit 1
fi

#---- Verzeichnis anlegen ----
TAN_DIR="TAN"
mkdir -p "$TAN_DIR"

#---- Ausgabedatei festlegen ----
TAN_FILE="$TAN_DIR/${USERNAME}.txt"

#---- 1) Wähle einen starken Zufallsseed ----
raw_seed=$(head -c 32 /dev/urandom | base64) #32 rohe Bytes Zufallsdaten, damit geht das in base64 kodiert beliebige Binärdaten in druckbaren ASCII-Text
#  Warum 32 Bytes, weil 32bytes = 256 Bit ist und sha256 braucht das ... -C  Druckt die Bytes jeder der angegebenen Dateien.

#---- 2) Optionales Salt pro User (hier Nutzername) ----
salt="$USERNAME"

#---- Seed kombinieren ----
seed="${salt}:${raw_seed}"

#---- 3) Erzeuge die Hash-Kette mit SHA-256 ----
#     chain[0] = seed, chain[i] = sha256(chain[i-1])
declare -a chain    #declare klar zu machen, dass chain ein Array ist (-a)
chain[0]="$seed"
for i in $(seq 1 $NUM_TANS); do   ##seq erzeugt eine fortlaufende Liste von Zahlen. ähnlich wie for(int i: 3); die schleife läuft dann NUM_TANS mal
  chain[$i]=$(printf "%s" "${chain[$((i-1))]}" | sha256sum | cut -d ' ' -f1) #printf formatiert den String (Keine Leerzeich uws..), sha256sum berechnet den SHA-256 Hash und cut extrahiert nur den Hash-Wert
done

#---- 4) Schreibe die TAN-Liste in umgekehrter Reihenfolge: h^n(seed) ... h^1(seed) ----
echo "# TAN-Liste für $USERNAME" > "$TAN_FILE"
for i in $(seq $NUM_TANS -1 1); do  
  echo "${chain[$i]}" >> "$TAN_FILE"
done

#---- Ausgabe fertig ----
echo "Generierte $NUM_TANS TANs (SHA-256+Salt) für Benutzer '$USERNAME'."
echo "Die TANs findest du in $TAN_FILE"
bat TAN/${USERNAME}.txt #ist wie cat aber nur cooler :)

# Hinweis:
# SHA-256 bietet eine gute Balance aus Geschwindigkeit und Sicherheit.
# Der Salt (Username) erhöht die Einzigartigkeit pro Nutzer.
