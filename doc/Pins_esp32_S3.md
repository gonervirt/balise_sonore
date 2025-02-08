# Configuration des broches ESP32 

## Broches de communication

| Broche | Type | Fonction | Description |
|--------|------|----------|-------------|
| 18 (RXD2) | Entrée | Communication MP3 | Réception série pour le module MP3 |
| 17 (TXD2) | Sortie | Communication MP3 | Transmission série pour le module MP3 |

## Broches de contrôle

| Broche | Type | Fonction | Description |
|--------|------|----------|-------------|
| 15 | Entrée | Récepteur AM | Entrée pour le signal du récepteur |
| 2 | Sortie | LED Verte | LED "Message en cours" |
| 13 | Sortie | LED Jaune | LED "Demande de lecture du message" |

## Broches analogiques

| Broche | Type | Fonction | Description |
|--------|------|----------|-------------|
| 32 | Entrée | Potentiomètre Volume | Réglage du volume |
| 35 | Entrée | Potentiomètre Durée | Réglage du temps d'inhibition |

## Notes importantes

- Toutes les entrées numériques sont configurées avec des pull-up internes
- Les LEDs sont actives à l'état HAUT (HIGH)
- Les broches analogiques sont utilisées pour le réglage des paramètres de fonctionnement