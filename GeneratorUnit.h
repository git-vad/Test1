#ifndef GENERATOR_UNIT_H
#define GENERATOR_UNIT_H
/**
 * @file GeneratorUnit.h
 * @author vladimir_ad@mail.ru
 * @brief 
 * @version 0.1
 * @date 2025-03-05
 */

#include <main.h>
#include <functional>
#include "IGenerator.h"
#include "CBQueue.h"

using extern_func = std::function<void()>;
 
/**
 * @details TGenerator предназначен...  
 * 
 */
class TGenerator final : public generator::IGenerator
{
  public:
    explicit TGenerator(uint32_t clk, extern_func start, extern_func stop, extern_func lock, extern_func unlock)
    : clk_(clk), start_(start), stop_(stop), lock_(lock), unlock_(unlock), status_(generator::STATUS::STOPPED)
    {
    }
        
    //! \details Метод добавляет последовательность структур типа \a Packet
    generator::RESULT addSequence(std::span<generator::Packet> sequence) override {
      generator::RESULT res = generator::RESULT::OK;
      if (queue_.free_space() > sequence.size()) {
        for (generator::Packet pkt : sequence) {
          queue_.push(pkt);
        }
      }
      else {
        res = generator::RESULT::FAIL;
      }
      return res;
    }

    //! \details Метод инициирует старт генерации импульсов
    generator::RESULT start() override {
      lock_();
      if (status_ == generator::STATUS::STOPPED)
      {
        start_();
        status_ = generator::STATUS::RUNNING;
      }
      unlock_();
      return generator::RESULT::OK;
    }

    //! \details Метод инициирует остановку генерации импульсов
    generator::RESULT stop() override {
      lock_();
      stop_();
      status_ = generator::STATUS::STOPPED;
      unlock_();
      return generator::RESULT::OK;
    }

    //! \details Метод возвращает статус контроллера движения
    generator::STATUS getStatus() override {
        return status_;
    }

    //! \details Метод возвращает максимальное количество структур \a Packet, 
    //! обрабатываемых контроллером движения
    int16_t getCapacity() override {
      return queue_.capacity();
    }

    //! \details Метод возвращает количество структур \a Packet которое 
    //! может быть добавлено.
    int16_t getSpace() override {
      return queue_.free_space();
    }

    /**
     * @brief Метод возвращает следующее значение счетчика на котором
     * должно быть прерывание по ссылке указанной в параметрах. В случае
     * если сгенерированы все импульсы актуального пакета, то выполняется
     * извлечение из очереди следующего пакета. Если очередь пуста, то 
     * метод возвращает generator::RESULT::FAIL. В случае успешного 
     * вычисления следующего значения возвращается generator::RESULT::OK
     * @return generator::STATUS
     */
    generator::RESULT getNextCCR(uint16_t &ccr) { 
      if(toggle_count_ == toggle_number_)
      {
        generator::Packet p;
        if (queue_.pop(p) == 0)
        {
          return generator::RESULT::FAIL;            
        }
        else{
          frequency_ = p.freq;
          toggle_number_ = p.n * 2;
          toggle_count_ = 0;
          ccr_old_value_ = ccr_value_;
        }
      }
      toggle_count_ +=1;
      ccr_value_ = ccr_old_value_ + (0x0000ffff & ((toggle_count_ * clk_ + frequency_/2)/frequency_));
      ccr = ccr_value_;
      return generator::RESULT::OK;
    }

  private:
    TCBQueue<generator::Packet, 10> queue_;
    uint32_t clk_;
    extern_func start_;
    extern_func stop_;
    extern_func lock_;
    extern_func unlock_;

    generator::STATUS status_;
    uint16_t ccr_old_value_;
    uint16_t ccr_value_;
    uint32_t toggle_number_;
    uint32_t toggle_count_;
    uint32_t frequency_;
    
};
#endif //GENERATOR_UNIT_H