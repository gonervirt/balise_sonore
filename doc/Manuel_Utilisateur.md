# Manuel d'Utilisation - Balise Sonore

Ce manuel vous guide dans la configuration et la gestion de votre Balise Sonore via son interface web. Aucune compétence technique avancée n'est requise.

---

## 1. Première Connexion

Pour accéder aux réglages de la balise, vous devez vous y connecter directement avec votre ordinateur ou votre smartphone via le WiFi.

1.  Mettez la balise sous tension.
2.  Ouvrez les paramètres WiFi de votre appareil.
3.  Recherchez le réseau nommé **`BALISESONORE`**.
4.  Connectez-vous avec le mot de passe par défaut : **`BaliseSonore_Betton_Mairie`**
5.  Une fois connecté, ouvrez votre navigateur internet (Chrome, Safari, Firefox, etc.).
6.  Dans la barre d'adresse, tapez : **`http://192.168.4.1`**

> **Note :** Si une page ne s'affiche pas automatiquement, assurez-vous de bien taper l'adresse complète.

!Capture d'écran de la page d'accueil
*Page d'accueil de l'interface*

---

## 2. Sécurisation (Configuration WiFi)

Il est **fortement recommandé** de modifier le mot de passe par défaut pour empêcher d'autres personnes de modifier les réglages.

1.  Cliquez sur le bouton **Config WiFi** dans le menu.
2.  Dans la section "Mettre à jour les paramètres WiFi" :
    *   **SSID** : Vous pouvez changer le nom du réseau (facultatif).
    *   **Mot de passe** : Effacez le mot de passe actuel et saisissez le vôtre.
3.  Cliquez sur **Enregistrer**.

> **Attention :** Après avoir cliqué sur Enregistrer, la connexion WiFi sera coupée. Vous devrez vous reconnecter au réseau de la balise avec votre **nouveau mot de passe**.

!Capture d'écran de la configuration WiFi

### 📝 Notez votre nouveau mot de passe ici :

| Paramètre | Votre valeur |
| :--- | :--- |
| **Nouveau Nom (SSID)** | `__________________________` |
| **Nouveau Mot de Passe** | `__________________________` |

---

## 3. Gestion des Messages Audio

La gestion des messages se fait en deux étapes : l'ajout du fichier audio sur la carte SD, puis la configuration sur l'interface web.

### Étape A : Préparation de la carte SD
Les fichiers audio doivent être au format **MP3**. Pour que la balise les reconnaisse, ils doivent être nommés avec 4 chiffres :
*   `0001.mp3` (Message 1)
*   `0002.mp3` (Message 2)
*   `0003.mp3` (Message 3)
*   ...

1.  Éteignez la balise et retirez la carte micro SD.
2.  Insérez-la dans votre ordinateur.
3.  Copiez vos fichiers MP3 à la racine de la carte en respectant la numérotation.
4.  Remettez la carte dans la balise et rallumez-la.

### Étape B : Configuration sur l'interface Web
Une fois les fichiers sur la carte, vous devez dire à la balise comment les utiliser.

1.  Allez dans le menu **Config Messages**.
2.  **Ajouter un message** : Cliquez sur le bouton "Ajouter un message" pour créer une nouvelle ligne dans la liste. Ajoutez autant de lignes que vous avez de fichiers (Ligne 1 pour `0001.mp3`, Ligne 2 pour `0002.mp3`, etc.).
3.  **Nommer les messages** : Dans les zones de texte, écrivez une description pour chaque message (ex: "Message Accueil", "Fermeture Exceptionnelle").
    *   *La case changera de couleur pour indiquer une modification non sauvegardée.*
4.  Cliquez sur **Sauvegarder les descriptions** pour enregistrer vos textes.

!Capture d'écran de la liste des messages

### Choisir le message à diffuser
Pour sélectionner quel message sera lu lorsque la balise est activée :
1.  Cochez la case ronde (bouton radio) à gauche du message souhaité.
2.  Cliquez sur le bouton **Définir comme actif**.
3.  La section "Paramètres du message actuel" en haut de page se mettra à jour.

---

## 4. Réglage du Volume

Vous pouvez ajuster le volume sonore directement depuis l'interface.

1.  Allez dans le menu **Config Messages**.
2.  Descendez jusqu'à la section **Contrôle du Volume**.
3.  Entrez une valeur entre **0** (silence) et **30** (maximum).
4.  Cliquez sur **Enregistrer le Volume**.

---

## 5. Maintenance

Si la balise ne répond plus correctement, vous pouvez la redémarrer à distance.

1.  Allez dans le menu **Config ESP32**.
2.  Cliquez sur **Redémarrer ESP32**.
3.  Attendez environ 30 secondes avant de vous reconnecter au WiFi.

!Capture d'écran de la page maintenance

---

*Besoin d'aide ? Contactez le support technique.*
*Version du document : 1.0*