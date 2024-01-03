# README - Sistema de Producción con IPC

Este repositorio contiene el código fuente de un sistema de producción implementado en C, utilizando la comunicación entre procesos (IPC) con memoria compartida, semáforos y colas de mensajes. El sistema simula una fábrica que produce pedidos a partir de materiales recibidos y atiende las solicitudes de los clientes.

## Estructura del Proyecto

El proyecto se divide en varios archivos fuente, cada uno correspondiente a una funcionalidad específica:

- **fabrica.c**: Gestiona la producción de pedidos y el estado de los semáforos y la memoria compartida.
- **materiales.c**: Provee materiales para la producción, actualizando los semáforos según las cantidades de materiales disponibles.
- **peticiones.c**: Envía solicitudes de pedidos a la fábrica, actualizando la memoria compartida y la cola de mensajes.
- **informador.c**: Informa periódicamente sobre el estado de la producción, utilizando la memoria compartida y los semáforos.

## Compilación y Ejecución

Para compilar el proyecto, utiliza el siguiente comando:

```bash
gcc fabrica.c -o fabrica -lpthread
gcc materiales.c -o materiales
gcc peticiones.c -o peticiones
gcc informador.c -o informador
```

Luego, ejecuta cada componente en una terminal diferente:

1. Terminal para la fábrica:

   ```bash
   ./fabrica clave
   ```

2. Terminal para los materiales:

   ```bash
   ./materiales clave [archivo_materiales]
   ```

3. Terminal para las peticiones:

   ```bash
   ./peticiones clave [archivo_peticiones]
   ```

4. Terminal para el informador:

   ```bash
   ./informador clave periodo
   ```

Asegúrate de reemplazar "clave", "archivo_materiales", "archivo_peticiones" y "periodo" con valores adecuados.

## Importante

- La fábrica, los materiales y las peticiones deben ejecutarse simultáneamente para simular la producción.
- Se utiliza la biblioteca de IPC y la biblioteca de hilos POSIX.
- Presiona Ctrl+C en cada terminal para detener y limpiar los procesos.

Este sistema demuestra la sincronización y comunicación entre procesos mediante IPC y puede ser utilizado como base para proyectos más complejos o como un ejemplo didáctico de programación concurrente en C.
