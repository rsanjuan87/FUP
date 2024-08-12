### modernizar el sistema de captura de notificaciones
- iniciar un server local por x puerto 
- hacer un fordward del puerto al device
- el apk-server con cada notificacion hace un post de la notificacion
- en el local recibe el post y realiza el mostrado de la notificacion

### modernizar captura de launchers
- iniciar un server local por un puerto
- hacer fordward del puerto al device
- el apk-server hace el post de los launchers
- el app local recibe el post y muestra los launchers

### acceso a los archivos locales desde el dispositivo

- si se monta un ftp en el local
- se hace un fordward del puerto al device
- en el device el apk-server brinda un contentresolver q monte el puerto