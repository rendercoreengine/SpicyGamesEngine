# SPICY ENGINE - DOCUMENTATION LUA

## Vue d'ensemble
Ce document contient toutes les fonctions, variables et constantes disponibles pour les scripts Lua dans Spicy Engine.

---

## FONCTIONS DE BASE

### Gestion des Objets

#### GetObjectID(name)
**Description:** Retourne l'ID d'un objet par son nom
**Paramètres:**
- `name` (string) : Nom de l'objet
**Retour:** Integer (ID de l'objet) ou nil si non trouvé
**Exemple:**
```lua
local cubeID = GetObjectID("Cube_0")
```

#### FindObjectByName(name)
**Description:** Alias pour GetObjectID
**Paramètres:**
- `name` (string) : Nom de l'objet
**Retour:** Integer (ID) ou nil

---

### Position

#### GetObjectPosition(objID)
**Description:** Récupère la position d'un objet
**Paramètres:**
- `objID` (integer) : ID de l'objet
**Retour:** x, y, z (3 nombres)
**Exemple:**
```lua
local x, y, z = GetObjectPosition(0)
print("Position: " .. x .. ", " .. y .. ", " .. z)
```

#### SetObjectPosition(objID, x, y, z)
**Description:** Définit la position d'un objet
**Paramètres:**
- `objID` (integer) : ID de l'objet
- `x, y, z` (numbers) : Nouvelles coordonnées
**Retour:** Rien
**Exemple:**
```lua
SetObjectPosition(0, 5.0, 2.0, 3.0)
```

#### MoveObject(objID, dx, dy, dz)
**Description:** Déplace un objet de façon relative
**Paramètres:**
- `objID` (integer) : ID de l'objet
- `dx, dy, dz` (numbers) : Déplacement relatif
**Retour:** Rien
**Exemple:**
```lua
MoveObject(0, 1.0, 0.5, 0.0)  -- Avance l'objet de 1 unité en X
```

#### TranslateObject(objID, dx, dy, dz)
**Description:** Alias pour MoveObject
**Paramètres:** Identiques à MoveObject

---

### Rotation

#### GetObjectRotation(objID)
**Description:** Récupère la rotation d'un objet
**Paramètres:**
- `objID` (integer) : ID de l'objet
**Retour:** pitch, yaw, roll (3 nombres en degrés)
**Exemple:**
```lua
local pitch, yaw, roll = GetObjectRotation(0)
```

#### SetObjectRotation(objID, pitch, yaw, roll)
**Description:** Définit la rotation d'un objet
**Paramètres:**
- `objID` (integer) : ID de l'objet
- `pitch` (number) : Rotation X en degrés
- `yaw` (number) : Rotation Y en degrés
- `roll` (number) : Rotation Z en degrés
**Retour:** Rien
**Exemple:**
```lua
SetObjectRotation(0, 45.0, 90.0, 0.0)
```

---

### Échelle

#### GetObjectScale(objID)
**Description:** Récupère l'échelle d'un objet
**Paramètres:**
- `objID` (integer) : ID de l'objet
**Retour:** x, y, z (3 nombres)
**Exemple:**
```lua
local sx, sy, sz = GetObjectScale(0)
```

#### SetObjectScale(objID, x, y, z)
**Description:** Définit l'échelle d'un objet
**Paramètres:**
- `objID` (integer) : ID de l'objet
- `x, y, z` (numbers) : Facteurs d'échelle
**Retour:** Rien
**Exemple:**
```lua
SetObjectScale(0, 2.0, 1.5, 2.0)
```

---

### Couleur

#### GetObjectColor(objID)
**Description:** Récupère la couleur d'un objet
**Paramètres:**
- `objID` (integer) : ID de l'objet
**Retour:** r, g, b (3 nombres entre 0.0 et 1.0)
**Exemple:**
```lua
local r, g, b = GetObjectColor(0)
```

#### SetObjectColor(objID, r, g, b)
**Description:** Définit la couleur d'un objet
**Paramètres:**
- `objID` (integer) : ID de l'objet
- `r, g, b` (numbers) : Valeurs RGB entre 0.0 et 1.0
**Retour:** Rien
**Exemple:**
```lua
SetObjectColor(0, 1.0, 0.0, 0.0)  -- Rouge
```

---

### Physique

#### SetObjectVelocity(objID, x, y, z)
**Description:** Définit la vélocité d'un objet
**Paramètres:**
- `objID` (integer) : ID de l'objet
- `x, y, z` (numbers) : Vecteur de vélocité
**Retour:** Rien
**Exemple:**
```lua
SetObjectVelocity(0, 0.0, 10.0, 0.0)
```

#### GetObjectVelocity(objID)
**Description:** Récupère la vélocité d'un objet
**Paramètres:**
- `objID` (integer) : ID de l'objet
**Retour:** x, y, z (3 nombres)
**Exemple:**
```lua
local vx, vy, vz = GetObjectVelocity(0)
```

#### ApplyForce(objID, fx, fy, fz)
**Description:** Applique une force à un objet (ajoute à la vélocité)
**Paramètres:**
- `objID` (integer) : ID de l'objet
- `fx, fy, fz` (numbers) : Force à appliquer
**Retour:** Rien
**Exemple:**
```lua
ApplyForce(0, 0.0, 50.0, 0.0)  -- Impulsion vers le haut
```

#### SetObjectPhysics(objID, enabled)
**Description:** Active/désactive la physique pour un objet
**Paramètres:**
- `objID` (integer) : ID de l'objet
- `enabled` (boolean) : true pour activer, false pour désactiver
**Retour:** Rien
**Exemple:**
```lua
SetObjectPhysics(0, true)
```

#### SetObjectStatic(objID, isStatic)
**Description:** Rend un objet statique (immobile)
**Paramètres:**
- `objID` (integer) : ID de l'objet
- `isStatic` (boolean) : true pour statique, false pour dynamique
**Retour:** Rien
**Exemple:**
```lua
SetObjectStatic(1, true)  -- Rend un objet immobile
```

---

### Visibilité

#### SetObjectVisible(objID, visible)
**Description:** Affiche ou cache un objet
**Paramètres:**
- `objID` (integer) : ID de l'objet
- `visible` (boolean) : true pour afficher, false pour cacher
**Retour:** Rien
**Exemple:**
```lua
SetObjectVisible(0, false)  -- Cache l'objet
```

---

### Utilitaires

#### GetDistance(objID1, objID2)
**Description:** Calcule la distance entre deux objets
**Paramètres:**
- `objID1, objID2` (integers) : IDs des deux objets
**Retour:** number (distance)
**Exemple:**
```lua
local distance = GetDistance(0, 1)
if distance < 5.0 then
    print("Les objets sont proches!")
end
```

#### GetObjectCount()
**Description:** Retourne le nombre total d'objets dans la scène
**Paramètres:** Aucun
**Retour:** Integer
**Exemple:**
```lua
local count = GetObjectCount()
print("Nombre d'objets: " .. count)
```

#### Print(message)
**Description:** Affiche un message dans la console
**Paramètres:**
- `message` (string) : Message à afficher
**Retour:** Rien
**Exemple:**
```lua
Print("Debug: Valeur x = 5.0")
```

---

### Clavier

#### IsKeyPressed(keyName)
**Description:** Vérifie si une touche est enfoncée
**Paramètres:**
- `keyName` (string) : Nom de la touche
**Retour:** boolean
**Touches supportées:**
- "W", "A", "S", "D"
- "Space", "Shift"
- "Up", "Down", "Left", "Right"
**Exemple:**
```lua
if IsKeyPressed("W") then
    Print("Touche W enfoncée!")
end
```

#### GetKey(keyName)
**Description:** Alias pour IsKeyPressed
**Paramètres:** Identiques à IsKeyPressed

---

## VARIABLES D'ENVIRONNEMENT

### Variables Globales

```lua
-- Ces variables sont disponibles dans tous les scripts

-- Identifiant de l'objet courant (défini automatiquement)
-- Accédé via les appels de script
```

---

## STRUCTURE D'UN SCRIPT

### Pattern Standard

```lua
-- Script attaché à un objet
function OnUpdate()
    -- Code exécuté à chaque frame
    
    -- Récupérer l'ID de l'objet courant
    local objID = GetObjectID("MaBoîte")
    
    -- Faire quelque chose
    if IsKeyPressed("Space") then
        SetObjectVelocity(objID, 0.0, 10.0, 0.0)
    end
end
```

### Fonction d'Entrée

La fonction `OnUpdate()` est appelée automatiquement chaque frame pour les objets qui ont un script attaché.

---

## EXEMPLES PRATIQUES

### Exemple 1 : Objet qui suit la souris

```lua
function OnUpdate()
    local objID = GetObjectID("Cube_0")
    
    if IsKeyPressed("W") then
        MoveObject(objID, 0.0, 0.0, -0.5)
    end
    if IsKeyPressed("S") then
        MoveObject(objID, 0.0, 0.0, 0.5)
    end
    if IsKeyPressed("A") then
        MoveObject(objID, -0.5, 0.0, 0.0)
    end
    if IsKeyPressed("D") then
        MoveObject(objID, 0.5, 0.0, 0.0)
    end
end
```

### Exemple 2 : Rotation continue

```lua
function OnUpdate()
    local objID = GetObjectID("Cube_0")
    local pitch, yaw, roll = GetObjectRotation(objID)
    
    SetObjectRotation(objID, pitch, yaw + 2.0, roll)
end
```

### Exemple 3 : Changement de couleur sur proximité

```lua
function OnUpdate()
    local cubeID = GetObjectID("Cube_0")
    local sphereID = GetObjectID("Sphere_0")
    
    local distance = GetDistance(cubeID, sphereID)
    
    if distance < 10.0 then
        SetObjectColor(cubeID, 1.0, 0.0, 0.0)  -- Rouge si proche
    else
        SetObjectColor(cubeID, 0.0, 0.0, 1.0)  -- Bleu si loin
    end
end
```

### Exemple 4 : Physique simple

```lua
function OnUpdate()
    local objID = GetObjectID("Ball")
    
    -- Activer la physique si pas déjà activée
    SetObjectPhysics(objID, true)
    SetObjectStatic(objID, false)
    
    -- Sauter avec la barre espace
    if IsKeyPressed("Space") then
        ApplyForce(objID, 0.0, 50.0, 0.0)
    end
end
```

### Exemple 5 : Masquer/Afficher

```lua
function OnUpdate()
    local objID = GetObjectID("Cube_0")
    
    if IsKeyPressed("H") then
        SetObjectVisible(objID, false)
    end
    if IsKeyPressed("S") then
        SetObjectVisible(objID, true)
    end
end
```

---

## TYPES DE DONNÉES

| Type | Description | Exemple |
|------|-------------|---------|
| integer | Nombre entier | `0, 1, -5, 100` |
| number | Nombre décimal | `1.5, 3.14, -0.5` |
| string | Texte | `"Cube_0", "Hello"` |
| boolean | Vrai/Faux | `true, false` |

---

## RESTRICTIONS ET LIMITATIONS

1. **Pas d'accès direct au système de fichiers** - Utilisez l'éditeur pour importer des ressources
2. **Pas de création d'objets depuis Lua** - Créez les objets via l'éditeur
3. **Pas d'accès aux shaders** - Les shaders sont définis dans l'éditeur
4. **Performance** - Les scripts lourds peuvent ralentir le moteur

---

## CONSEILS DE DÉVELOPPEMENT

- Utilisez `Print()` pour déboguer
- Groupez les objets apparentés avec la hiérarchie
- Testez avec le mode Live Test (F5)
- Évitez les appels à `GetObjectID()` en boucle, stockez l'ID dans une variable

---

## VERSION

Spicy Engine v1.0 - 2025
Documentation Lua - v1.0