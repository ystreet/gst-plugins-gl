//Pour avoir le mode debug de glib il faut définir la variable d'environnement G_DEBUG
//-> http://library.gnome.org/devel/glib/stable/glib-running.html

gst-launch-0.10 videotestsrc ! video/x-raw-rgb ! glimagesink

gst-launch-0.10 videotestsrc ! glgraphicmaker ! glimagesink

gst-launch-0.10 -v videotestsrc ! glgraphicmaker ! glimagesink

gst-launch-0.10 --gst-debug-level=3 videotestsrc ! glgraphicmaker ! glimagesink

videotestsrc num_buffers=200 ! video/x-raw-rgb, framerate = 25/1, width=320, height=240 !  tee name=teee teee. ! queue ! glimagesink teee. ! queue ! glimagesink teee. ! queue ! glimagesink

videotestsrc num_buffers=200 ! video/x-raw-yuv, format=(fourcc)I420, framerate = 25/1, width=320, height=240 !  tee name=t t. ! queue ! glimagesink t. ! queue ! glimagesink t. ! queue ! glimagesink

videotestsrc num_buffers=200 ! video/x-raw-yuv, format=(fourcc)YUY2, framerate = 25/1, width=320, height=240 !  tee name=t t. ! queue ! glimagesink t. ! queue ! glimagesink t. ! queue ! glimagesink

videotestsrc num_buffers=200 ! video/x-raw-yuv, format=(fourcc)YV12, framerate = 25/1, width=320, height=240 !  tee name=t t. ! queue ! glimagesink t. ! queue ! glimagesink t. ! queue ! glimagesink

videotestsrc num_buffers=200 ! video/x-raw-rgb, framerate = 25/1, width=320, height=240 !  tee name=teee teee. ! queue ! glgraphicmaker ! glimagesink teee. ! queue ! glgraphicmaker ! glimagesink teee. ! queue ! glgraphicmaker ! glimagesink




Idée pour faire un pool de textures: (considérations pour une texture "rgb", "yuv" étant une généralisation)

Dans un programme opengl normal, on appel une unique fois glGenTextures (couteux) puis on l'actualise à chaque fois 
avec glTexImage2D.
Donc dans ce cas la consommation de ressources est optimale.

Cependant dans le context de graph, le glbuffer, qui contient une texture, est sujet à passer de filtre en filtre.
Et donc à priori on ne s'est pas à l'avance ce qu'il va devenir. Ce qui implique que l'on est obligé d'appeler
glGenTextures à chaque nouvelle frame. C'est ce qui est fait actuellement.

Un pool de textures permettrait donc de diminuer considérablemet le nombre d'appels à glGenTextures.
Cependant un pool de textures simple, c'est à dire avec un nombre fixe d'appels à glGenTextures n'est pas
réalisable.
En effet, du fait qu'on ne s'est pas à l'avance ce que vont devenir les glbuffers, il est impossible de déterminer
la taille minimale du pool.

Mais dans l'architechture de ce plugin opengl, on a la chance d'avoir un contexte commun à toute une branche 
de la pipeline, le GstGLDisplay. (A noter que le "contexte" commun à toute ces branches est la map)

De ce fait je propose la chose suivante:

Le pool est constitué d'une Queue de GLuint.
Il y aura un pool par contexte GstGLDisplay.
Au départ le pool est de taille 0. 
A chaque nouvelle frame, si le pool est de taille 0 alors le glbuffer crée fait appel à un glGenTextures. 
Lorsque le buffer est détruit, il le signal au contexte GstGLDisplay qui lui va ajouter à la Queue le GLuint utilisé.
Lorsqu'une nouvelle frame arrive et que le pool n'est pas de taille 0, le glbuffer crée, pop la Queue et utilise le GLuint 
directement sans faire appel à glGenTextures.
A la fin du stream, plus aucune frame arrive et donc plus aucun glbuffer est crée.
Les glbuffer qui naviguaient dans la pipeline sont peu à peu détruits, ce qui augmente peu à peu la taille de la Queue.
Si le stream est à nouveau joué, on fait pareil que décrit précédemment.
Si la branche de la pipeline est totalement détruite, le compteur de référence de son contexte GstGLDisplay atteint 0, 
le destructeur est appelé et on vide la Queue en tachant de faire un glDeleteTextures à pour chaque GLuint popé.

Affin de géré le "yuv", il y aura non pas une Queue de GLuint mais une Queue de triplé GLuint