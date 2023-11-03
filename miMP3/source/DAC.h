
#ifndef DAC_H_
#define DAC_H_

#include "hardware.h"


typedef DAC_Type *DAC_t;
typedef uint16_t DACData_t;

/**
 * @brief Inicialización de todo lo que necesita DAC.
 */
void init_dac	 (void);
void DAC_Pause	 (void);
/**
 * @brief se envia lo que se va convertir en analogico.
 * @param data data a enviar
 */
void DAC_SetData( DACData_t data );

/**
 * @brief 
 */
void DAC_PISR (void);

/**
 * @brief Se genera una onda triangular
 * @param pico pico de tension del triangulo, 3.3 por default
 * @param paso resolucion
 */
void Build_trinangle(uint32_t pico,int paso); //

/**
 * @brief Se genera una onda triangular
 * @param pico pico de tension del triangulo, 3.3 por default
 * @param paso resolucion
 */
void Build_sine(int resolucion,int kfrecuencia); //

#endif /* void DAC_H_ */
