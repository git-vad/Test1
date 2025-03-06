#ifndef GENERATOR_H
#define GENERATOR_H
/**
 * @file Generator.h
 * @author vladimir_ad@mail.ru
 * @brief Файл содержит объявления для Генератора
 * @version 0.1
 * @date 2025-03-05
 */

#include "IGenerator.h"

extern generator::IGenerator *pGenerator;

void GeneratorInit(void);

#endif //GENERATOR_H