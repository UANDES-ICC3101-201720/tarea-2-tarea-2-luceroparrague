# virtmem
Operating Systems and Networks - Virtual Memory Assignment (Tarea 2)

INFORME TAREA 2

	miembros:
		-Vicente Lucero
		-Ignacio Parrague
		
	La realizacion de esta tarea, consistio en la implementacion de tres diferentes algoritmos de manejos de falla de paginas al momento de leer o escribir en un disco de memoria virtual. Estos algoritmos son FIFO (first in first out), un random (que escoja el lugar de reemplazo al azar) y un algoritmo de nuestra eleccion y/o creacion, en este caso, se decidio realizar un algoritmo que se alterne entre random y FIFO para realizar el reemplazo.
	
	Antes de realizar cualquier iteracion para la obtencion de datos, expondremos nuestras hipotesis respecto a cada uno de los algoritmos y su comportamiento.
	
	FIFO: Siempre el tiempo de realizacion deberia ser siempre constante debido a que siempre debe buscar el primer marco ingresado disponible para asignar los datos necesarios para la realizacion de las intrucciones utilizadas en el proceso, ya que su objetivo es guardar las paginas en el orden que fueron cargadas. En teoria, por lo visto en clases, la cantidad de faltas de paginas deberia disminuir a medida que aumentan los marcos disponibles en la memoria virtual. Sin embargo, es sabido que en este tipo de reemplazo existe la llamada "Anomalia de Belady" donde se produce un fenomeno de aumento de faltas al pasar de tres a cuatro marcos, por lo cual es esperable que sea reflejado en este experimento. 
	
	Random: El tiempo de realizacion se espera sea variado debido a la impredicibilidad con que la pagina escogida para el reemplazo sea valida o si esta puede causar una nueva falta. Lo mismo ocurre con la comparacion de faltas vs marcos, sin embargo, considerando que en general un mayor numero de marcos implica una menor cantidad de faltas por la disponibilidad de las mismas, deberia disminuir la cantidad de faltas de estas mismas. Por otro lado, siempre esta el caso en que los numeros generados tengan el caso de necesitar demaciados intentos para lograr un reemplazo, por lo cual la comparacion depende de un poco de la suerte.
	
	Custom: Dado que es una mezcla de ambos algoritmos, y que a la vez cual algortimo es escogido depende de un random entre cero y uno, el tiempo de reemplazo va a variar de diversas formas. El mejor de los casos es que siempre escogiera el FIFO, donde el tiempo de realizacion y la cantidad de faltas vs marcos seran exactamente las mismas. El peor de los casos sera cuando unicamente sea escogido el random causando las mismas condiciones de fifo pero analogas a random.
	
	En general es esperado que la cantidades de fallas disminuyan a medida que aumentemos la cantidad de marcos disponibles. 
	
	
