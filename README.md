# cRelay

Exécuter la ligne de commmande présente dans le fichier TODO.txt

Compilation
  - make
  
Déploiement de l'exécutable sans toucher au fichier de configuration déloyé précédement
  - sudo make install
  
Déploiement de l'exécutable avec déploiement du fichier de configuration
  - sudo make install CONF=conf/<myfile>.conf

lancement en tâche de fond.
  - /usr/local/bin/crelay -D
  
Lancement au démarrage du raspberry PI
  - Ajouter la ligne ci-dessus à la fin du fichier (avant la ligne "exit(0)" bien sûr)
      * sudo vi /etc/rc.local
  
