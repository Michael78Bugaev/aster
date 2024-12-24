#include <signal.h>
#include <stdio.h>

sighandler_t signal_handlers[32]; // Массив обработчиков сигналов

sighandler_t signal(int signum, sighandler_t handler) {
    if (signum < 0 || signum >= 32) {
        return SIGTERM; // Ошибка: неверный номер сигнала
    }
    sighandler_t old_handler = signal_handlers[signum];
    signal_handlers[signum] = handler; // Установка нового обработчика
    return old_handler; // Возвращаем старый обработчик
}

void raise(int signum) {
    if (signum < 0 || signum >= 32) {
        return; // Ошибка: неверный номер сигнала
    }
    if (signal_handlers[signum]) {
        signal_handlers[signum](signum); // Вызов обработчика сигнала
    }
}