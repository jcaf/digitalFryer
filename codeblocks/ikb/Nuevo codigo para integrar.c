
|
|
|
___________________________________ 500 ms (en este punto el programa pregunta)
|
|  _-_                   _-_
|
| hardware fue pulsado

pregunta @ 500ms:
if (esta siendo pulsado todavia osea es mayor que 500ms...)
{
    se apropia del grupo y se queda con el boton, el otro se deshabilita
}
else
{
    if ( si termino <500 ms) && (el otro No esta pulsado)
    {
        se toma como una pulsacion simple
    }
    else
    {
        entre sus primeros 500 ms el otro tambien esta pulsado, entonces ambos
        entran en el modo de esperar hasta 3segundos para que sea tomado como una
        combinacion

        Si alguno de ellos deja de ser pulsado, todo queda en nada y se espera un nuevo ciclo para que ambos
        esten sin presionar...
        //
        EN ESTE PUNTO AMBOS SE VAN A UN "MUNDO" DE ESPERA HACIENDO UN "AND"... ALLI EL ULTIMO
        DE ELLOS SE INTEGRA E INICIA RECIEN LA CUENTA... RESTANDO DEL ULTIMO -0.5s

        thiskey.flag = 1;
    }    
}

.
..
...
....
else if (sm0 == MUNDO)
{
    /*ESPERANDO EL TOTAL CON UN AND*/
    if ( thiskey1.flag &&  thiskey1.flag) // tal vez esto va a hacer hecho a nivel de usuario
    {
        //entraron en este flag

    }


}

