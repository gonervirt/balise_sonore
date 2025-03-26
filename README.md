# Balise Sonore ESP32

## Description
Système de balise sonore basé sur ESP32, conçu pour diffuser des messages audio pré-enregistrés en réponse à un bouton poussoir ou à une commande radio. Le système est configurable via une interface web et peut fonctionner en mode point d'accès (AP) ou en mode station (STA).

## Caractéristiques
- Compatible avec plusieurs cartes ESP32 (C3 Mini, S2, S3)
- Lecture de fichiers audio via DFPlayer Mini
- Interface web de configuration
- Déclenchement par bouton poussoir ou commande radio
- Indication visuelle par LED RGB
- Système de fichiers LittleFS pour le stockage
- Mode point d'accès WiFi ou connexion à un réseau existant

## Matériel nécessaire
- Une carte ESP32 compatible (LOLIN C3 Mini, ESP32-S2, ou ESP32-S3)
- Module DFPlayer Mini
- Haut-parleur 3W minimum
- Bouton poussoir
- LEDs (rouge, vert, jaune) ou LED RGB
- Module récepteur radio 433MHz (optionnel)
- Carte micro SD (pour les fichiers audio)

## Configuration des broches
### LOLIN C3 Mini
- RX/TX DFPlayer : 20/21
- Bouton : 6
- LED Verte : 2
- LED Jaune : 1
- LED Rouge : 0
- Radio : 9

### ESP32-S2
- RX/TX DFPlayer : 17/18
- Bouton : 38
- LED Verte : 35
- LED Jaune : 36
- LED Rouge : 37
- Radio : 33

### ESP32-S3
- RX/TX DFPlayer : 15/16
- Bouton : 14
- LED Verte : 11
- LED Jaune : 12
- LED Rouge : 13
- Radio : 10

## Installation
1. Cloner le dépôt
2. Ouvrir le projet dans PlatformIO
3. Sélectionner l'environnement correspondant à votre carte
4. Compiler et téléverser

## Configuration
1. À la première utilisation, la balise crée un point d'accès WiFi
   - SSID: "BALISESONORE"
   - Mot de passe: "BaliseSonore_Betton_Mairie"
2. Se connecter à l'interface web (http://192.168.4.1)
3. Configurer les paramètres :
   - Mode WiFi (AP/STA)
   - SSID et mot de passe du réseau
   - Numéro du message audio

## États de fonctionnement
1. **STARTING** : Démarrage initial (LED jaune)
2. **READY_WAITING** : Prêt à diffuser (LED verte)
3. **PLAYING_TONE** : Diffusion en cours (LED jaune)
4. **INHIBITED** : Période de repos après diffusion (LED rouge)

## Fichiers audio
- Format : MP3
- Stockage : Carte micro SD
- Numérotation : 0001.mp3, 0002.mp3, etc.

