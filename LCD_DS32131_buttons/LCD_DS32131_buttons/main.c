/*
 * LCD_DS32131_buttons.c
 *
 * Created: 29.02.2020 19:27:55
 * Author : GaD_Bogdan
 */ 

#include "main.h"
//===================================================================================================================
#define  delay 100

#define deviceAddress 0b11010000			// Адрес DS3231 в шине I2C
#define Button_PORT PORTD					// Порт, на котором весят кнопки
#define First_Button_PIN 5					// Пин первой кнопки
#define Second_Button_PIN 6					// Пин второй кнопки
#define Third_Button_PIN 7					// Пин третьей кнопки

typedef enum					// Объявляем перечисляемый тип данных
{
	Clock_Mode_common,			// Обыяный режим, ни одна из кнопок не нажата
	Clock_Mode_hour,			// Работа с часами
	Clock_Mode_min,				// Работа с минутами
	Clock_Mode_sec,				// Работа с секундами
	Clock_Mode_day,				// Работа с днём недели
	Clock_Mode_date,			// Работа с датой
	Clock_Mode_month,			// Работа с месяцем
	Clock_Mode_year,			// Работа с годом
} Clock_Modes;					// Называем этот тип Clock_Modes
//===================================================================================================================
void LCD_Show_day(byte days);    // Функция для вывода дня недели
void LCD_send_BCD(byte data);	 // Функция для вывода двоично-десятичного кода
void Button_PORT_ini(void);      // Функция для инициализации пинов с кнопками
void Modify_Value_of_RTC(byte index, byte direction); // Функция для обновления показываемых значений
//===================================================================================================================
unsigned char sec, min, hour, days, date, month, year;	/* Заводим новые переменные, для каждого
														  вида используемых значений */
Clock_Modes Clock_Mode = Clock_Mode_common;				// Заводим переменную объявленного выше типа данных
byte First_button_state = 0;
byte blinkstate = 0;
//===================================================================================================================
int main(void)
{
    LCD_init();									// Инициализация LCD
	I2C_Init();									// Инициализация I2C
	Button_PORT_ini();							// Инициализация порта с кнопками
	
// 	I2C_StartCondition();
// 	I2C_SendByte(deviceAddress);             // Говорим нашему счетчику, что мы обращаемся к нему
// 	I2C_SendByte(4);                         // Выбираем самый первый аддрес (в нем хранятся секунды)
// 	I2C_SendByte(RTC_ConvertFromDecToBD(29));
// 	I2C_StopCondition();
	
    while (1) 
    {
		_delay_ms(delay);
//===================================================================================================================
		I2C_SendByteByAddr(0, deviceAddress);			// Переходим на адрес 0, обращаясь при этом к адресу нашего RTC
		I2C_StartCondition();							// Условие старт для шини TWI
		I2C_SendByte((deviceAddress | 1));				// Говорим RTC что мы обращаемся к нему и что хотим читать
				
		sec = I2C_ReadByte();							// Читаем секунды, которые нам передают с 0 адреса
		min = I2C_ReadByte();							// Здесь адрес проинкрементился и теперь равен 1, там минуты
		hour = I2C_ReadByte();							// Часы
		days = I2C_ReadByte();							// День недели
		date = I2C_ReadByte();							// Дата
		month = I2C_ReadByte();							// Месяц
		year = I2C_ReadLastByte();						// Год
		I2C_StopCondition();							// Условие стоп
//===================================================================================================================
		if(!(PIN(Button_PORT) & (1 << First_Button_PIN))) // Кнопка 1 нажака
		{
			if (Clock_Mode == Clock_Mode_common)
			{
				Clock_Mode = Clock_Mode_hour;		// Перейдем в режим перевода часов
				blinkstate = 0;						// Сбросим счетчик мегания
				First_button_state = 1;				// Кнопка один нажата
			}
		}
//===================================================================================================================
		LCD_set_pos(4,0);                            // Перемещаем указатель на середину первой строки
//===================================================================================================================
		if (Clock_Mode != Clock_Mode_hour)           // Если мы не находимся в режиме изменения часов
		{
			LCD_send_BCD(hour);						// То просто выводим часы
		}	
		else										// Если нет, то...
		{
			if (blinkstate == 0)					// Если флаг blinkstate  в нуле
			{
				LCD_send_char(' ');					// Выводим пустые символы
				LCD_send_char(' ');					
				blinkstate = 1;						// И выставляем флаг в 1
			} 
			else
			{
				LCD_send_BCD(hour);					// А если флаг был в единице, то выводим часы
				blinkstate = 0;						// И сбрасываем флаг
			}
			if(!(PIN(Button_PORT) & (1 << First_Button_PIN))) // Кнопка 1 нажата
			{
				if (First_button_state == 0) // Опросим статус, чтобы сразу не перейти в режим перевода минут
				{
					Clock_Mode = Clock_Mode_min;	// Режим перевода минут
					First_button_state = 1;			// Поставим опять флаг состояния первой кнопки в единицу
				}
			}
			if (Clock_Mode == Clock_Mode_hour) First_button_state = 0;					// Сбросим статус первой кнопки
			if(!(PIN(Button_PORT) & (1 << Second_Button_PIN))) Modify_Value_of_RTC(1,0); //  Кнопка 2 нажата
			if(!(PIN(Button_PORT) & (1 << Third_Button_PIN))) Modify_Value_of_RTC(1,1); //  Кнопка 3 нажата
		}
//===================================================================================================================		
		LCD_send_char(':');				// Разделим часы и минуты
//===================================================================================================================		
		if (Clock_Mode != Clock_Mode_min)           // Если мы не находимся в режиме изменения минут
		{
			LCD_send_BCD(min);						// То просто выводим минуты
		}
		else										// Если нет, то...
		{
			if (blinkstate == 0)					// Если флаг blinkstate  в нуле
			{
				LCD_send_char(' ');					// Выводим пустые символы
				LCD_send_char(' ');
				blinkstate = 1;						// И выставляем флаг в 1
			}
			else
			{
				LCD_send_BCD(min);					// А если флаг был в единице, то выводим минуты
				blinkstate = 0;						// И сбрасываем флаг
			}
			if(!(PIN(Button_PORT) & (1 << First_Button_PIN))) // Кнопка 1 нажата
			{
				if (First_button_state == 0) // Опросим статус, чтобы сразу не перйти в режим перевода секунд
				{
					Clock_Mode = Clock_Mode_sec;	// Режим перевода секунд
					First_button_state = 1;
				}
			}
			if (Clock_Mode == Clock_Mode_min) First_button_state = 0; //Сбросим статус
			if(!(PIN(Button_PORT) & (1 << Second_Button_PIN))) Modify_Value_of_RTC(2,0); //  Кнопка 2 нажата
			if(!(PIN(Button_PORT) & (1 << Third_Button_PIN))) Modify_Value_of_RTC(2,1); //  Кнопка 3 нажата
		}
//===================================================================================================================
		LCD_send_char(':');				// Разделим минуты и секунды
//===================================================================================================================
		if (Clock_Mode != Clock_Mode_sec)           // Если мы не находимся в режиме изменения секунд
		{
			LCD_send_BCD(sec);						// То просто выводим секунды
		}
		else										// Если нет, то...
		{
			if (blinkstate == 0)					// Если флаг blinkstate  в нуле
			{
				LCD_send_char(' ');					// Выводим пустые символы
				LCD_send_char(' ');
				blinkstate = 1;						// И выставляем флаг в 1
			}
			else
			{
				LCD_send_BCD(sec);					// А если флаг был в единице, то выводим секунды
				blinkstate = 0;						// И сбрасываем флаг
			}
			if(!(PIN(Button_PORT) & (1 << First_Button_PIN))) // Кнопка 1 нажата
			{
				if (First_button_state == 0) // Опросим статус, чтобы сразу не перйти в режим перевода дня недели
				{
					Clock_Mode = Clock_Mode_day;	// Режим перевода дня недели
					First_button_state = 1;
				}
			}
			if (Clock_Mode == Clock_Mode_sec) First_button_state = 0; //Сбросим статус
			if(!(PIN(Button_PORT) & (1 << Second_Button_PIN))) Modify_Value_of_RTC(3,0); //  Кнопка 2 нажата
			if(!(PIN(Button_PORT) & (1 << Third_Button_PIN))) Modify_Value_of_RTC(3,1); //  Кнопка 3 нажата
		}
//===================================================================================================================
		LCD_set_pos(0,1);			// Переходм на начало второй строки дисплея
//===================================================================================================================
		if (Clock_Mode != Clock_Mode_day)           // Если мы не находимся в режиме изменения дня недели
		{
			LCD_Show_day(days);						// То просто выводим день недели
		}
		else										// Если нет, то...
		{
			if (blinkstate == 0)					// Если флаг blinkstate  в нуле
			{
				LCD_send_char(' ');					// Выводим пустые символы
				LCD_send_char(' ');
				LCD_send_char(' ');
				blinkstate = 1;						// И выставляем флаг в 1
			}
			else
			{
				LCD_Show_day(days);					// А если флаг был в единице, то выводим день недели
				blinkstate = 0;						// И сбрасываем флаг
			}
			if(!(PIN(Button_PORT) & (1 << First_Button_PIN))) // Кнопка 1 нажата
			{
				if (First_button_state == 0) // Опросим статус, чтобы сразу не перйти в режим перевода даты
				{
					Clock_Mode = Clock_Mode_date;	// Режим перевода даты
					First_button_state = 1;
				}
			}
			if (Clock_Mode == Clock_Mode_day) First_button_state = 0; //Сбросим статус
			if(!(PIN(Button_PORT) & (1 << Second_Button_PIN))) Modify_Value_of_RTC(4,0); //  Кнопка 2 нажата
			if(!(PIN(Button_PORT) & (1 << Third_Button_PIN))) Modify_Value_of_RTC(4,1); //  Кнопка 3 нажата
		}
//===================================================================================================================
		LCD_set_pos(6,1);			// Переходим на шестой символ второй строки
//===================================================================================================================
		if (Clock_Mode != Clock_Mode_date)           // Если мы не находимся в режиме изменения даты
		{
			LCD_send_BCD(date);						// То просто выводим дату
		}
		else										// Если нет, то...
		{
			if (blinkstate == 0)					// Если флаг blinkstate  в нуле
			{
				LCD_send_char(' ');					// Выводим пустые символы
				LCD_send_char(' ');
				blinkstate = 1;						// И выставляем флаг в 1
			}
			else
			{
				LCD_send_BCD(date);					// А если флаг был в единице, то выводим дату
				blinkstate = 0;						// И сбрасываем флаг
			}
			if(!(PIN(Button_PORT) & (1 << First_Button_PIN))) // Кнопка 1 нажата
			{
				if (First_button_state == 0) // Опросим статус, чтобы сразу не перйти в режим перевода месяцев
				{
					Clock_Mode = Clock_Mode_month;	// Режим перевода месяцев
					First_button_state = 1;
				}
			}
			if (Clock_Mode == Clock_Mode_date) First_button_state = 0; //Сбросим статус
			if(!(PIN(Button_PORT) & (1 << Second_Button_PIN))) Modify_Value_of_RTC(5,0); //  Кнопка 2 нажата
			if(!(PIN(Button_PORT) & (1 << Third_Button_PIN))) Modify_Value_of_RTC(5,1); //  Кнопка 3 нажата
		}
//===================================================================================================================
		LCD_send_char('.');			// Разделим дату и месяц точкой
//===================================================================================================================
		if (Clock_Mode != Clock_Mode_month)           // Если мы не находимся в режиме изменения месяца
		{
			LCD_send_BCD(month);						// То просто выводим месяц
		}
		else										// Если нет, то...
		{
			if (blinkstate == 0)					// Если флаг blinkstate  в нуле
			{
				LCD_send_char(' ');					// Выводим пустые символы
				LCD_send_char(' ');
				blinkstate = 1;						// И выставляем флаг в 1
			}
			else
			{
				LCD_send_BCD(month);					// А если флаг был в единице, то выводим месяц
				blinkstate = 0;						// И сбрасываем флаг
			}
			if(!(PIN(Button_PORT) & (1 << First_Button_PIN))) // Кнопка 1 нажата
			{
				if (First_button_state == 0) // Опросим статус, чтобы сразу не перйти в режим перевода года
				{
					Clock_Mode = Clock_Mode_year;	// Режим перевода года
					First_button_state = 1;
				}
			}
			if (Clock_Mode == Clock_Mode_month) First_button_state = 0; //Сбросим статус
			if(!(PIN(Button_PORT) & (1 << Second_Button_PIN))) Modify_Value_of_RTC(6,0); //  Кнопка 2 нажата
			if(!(PIN(Button_PORT) & (1 << Third_Button_PIN))) Modify_Value_of_RTC(6,1); //  Кнопка 3 нажата
		}
//===================================================================================================================
		LCD_send_char('.');		// Разделим месяц и год точкой
//===================================================================================================================
		if (Clock_Mode != Clock_Mode_year)           // Если мы не находимся в режиме изменения года
		{
			LCD_send_BCD(0b00100000);						// То выводим 20 и
			LCD_send_BCD(year);						// год
		}
		else										// Если нет, то...
		{
			if (blinkstate == 0)					// Если флаг blinkstate  в нуле
			{
				LCD_send_char(' ');					// Выводим пустые символы
				LCD_send_char(' ');
				LCD_send_char(' ');
				LCD_send_char(' ');
				blinkstate = 1;						// И выставляем флаг в 1
			}
			else
			{
				LCD_send_BCD(0b00100000);					// А если флаг был в единице, то выводим 20 и
				LCD_send_BCD(year);							// год
				blinkstate = 0;
			}
			if(!(PIN(Button_PORT) & (1 << First_Button_PIN))) // Кнопка 1 нажата
			{
				if (First_button_state == 0) // Опросим статус, чтобы сразу не перйти в общий режим
				{
					Clock_Mode = Clock_Mode_common;	// Общий режим
					First_button_state = 1;
				}
			}
			if (Clock_Mode == Clock_Mode_year) First_button_state = 0; //Сбросим статус
			if(!(PIN(Button_PORT) & (1 << Second_Button_PIN))) Modify_Value_of_RTC(7,0); //  Кнопка 2 нажата
			if(!(PIN(Button_PORT) & (1 << Third_Button_PIN))) Modify_Value_of_RTC(7,1); //  Кнопка 3 нажата
		}
//===================================================================================================================
    }
}
//===================================================================================================================
void LCD_Show_day(byte days)	// Функция которая выводит название дня недели
{
	switch (days)				// В зависимисти от переданного ей номера дня недели выводит его название
	{
		case 1:
		LCD_send_string("Mon");		// Понедельние
		break;
		case 2:
		LCD_send_string("Tue");		// Вторник
		break;
		case 3:
		LCD_send_string("Wed");		// Среда
		break;
		case 4:
		LCD_send_string("Thu");		// Четверг
		break;
		case 5:
		LCD_send_string("Fri");		// Пятница
		break;
		case 6:
		LCD_send_string("Sat");		// Суббота
		break;
		case 7:
		LCD_send_string("Sun");		// Воскресенье
		break;
	}
}
//===================================================================================================================
void LCD_send_BCD(byte data)				// Отправляем на LCD ДДК в виде обычных цифр
{
	LCD_send_char((data>>4) + 0x30);			// Добавляем 0x30, так как символ 0 имеет кодировку 0x30, символ 1 это 0x31 и т.д.
	LCD_send_char((data & 0x0F) + 0x30);
}
//===================================================================================================================
void Button_PORT_ini(void)				// Инициализация пинов с кнопками
{
	DDR(Button_PORT) &= ~((1<<First_Button_PIN) | (1<<Second_Button_PIN) | (1<<Third_Button_PIN)); // на вход
	Button_PORT |= ((1<<First_Button_PIN) | (1<<Second_Button_PIN) | (1<<Third_Button_PIN)); // подтягиваем к 1
}
//===================================================================================================================
void Modify_Value_of_RTC(byte index, byte direction)
{
	I2C_StartCondition();                // Условие старт, мы же будем писать в DS3231 наше измененное время сразу
	I2C_SendByte(deviceAddress);		 // Адрес устройства и "запись"
	switch(index)						 // В зависимости от того, в каком моде произошел вызов и что мы будем менять
	{
		case 1: // часы
			I2C_SendByte(2);             // Адрес часов 0x02
			hour = RTC_ConvertFromBDToDec(hour);  // Сперва приведем часы к нормальному значению, чтобы инкрементировать их
			if (direction)						  // Направление, в зависимости от того, какую кнопку нажимаем
			{
				if(hour < 23) I2C_SendByte(RTC_ConvertFromDecToBD(++hour));		// Если < 23 то прибавляем 1 и пишем в RTC
				else I2C_SendByte(RTC_ConvertFromDecToBD(1));					// Если нет, то пишем 1
			}
			else
			{
				if(hour > 1) I2C_SendByte(RTC_ConvertFromDecToBD(--hour));		// Если > 1 то отнимаем 1 и пишем в RTC
				else I2C_SendByte(RTC_ConvertFromDecToBD(23));					// Если нет, то пишем 23
			}
			break;
		case 2: // минуты 
			I2C_SendByte(1);             // Адрес минут 0x01
			min = RTC_ConvertFromBDToDec(min);	// Сперва приведем минуты к нормальному значению, чтобы инкрементировать их
			if (direction)						  // Направление, в зависимости от того, какую кнопку нажимаем
			{
				if(min < 59) I2C_SendByte(RTC_ConvertFromDecToBD(++min));		// Если < 59 то прибавляем 1 и пишем в RTC
				else I2C_SendByte(RTC_ConvertFromDecToBD(1));					// Если нет, то пишем 1
			}
			else
			{
				if(min > 1) I2C_SendByte(RTC_ConvertFromDecToBD(--min));		// Если > 1 то отнимаем 1 и пишем в RTC
				else I2C_SendByte(RTC_ConvertFromDecToBD(59));					// Если нет, то пишем 59
			}
			break;
		case 3: // секунды
			I2C_SendByte(0);             // Адрес секунд 0x00
			if (direction)							// Направление, в зависимости от того, какую кнопку нажимаем
			{
				I2C_SendByte(0);					// Пишем 0 секунд
			}
			else
			{
				I2C_SendByte(RTC_ConvertFromDecToBD(30));	// Пишем 30 секунд
			}
			break;
		case 4: // дни недели
			I2C_SendByte(3);             // Адрес недели 0x03
			days = RTC_ConvertFromBDToDec(days);  // Сперва приведем недели к нормальному значению, чтобы инкрементировать их
			if (direction)						  // Направление, в зависимости от того, какую кнопку нажимаем
			{
				if(days < 7) I2C_SendByte(RTC_ConvertFromDecToBD(++days));		// Если < 7 то прибавляем 1 и пишем в RTC
				else I2C_SendByte(RTC_ConvertFromDecToBD(1));					// Если нет, то пишем 1
			}
			else
			{
				if(days > 1) I2C_SendByte(RTC_ConvertFromDecToBD(--days));		// Если > 1 то отнимаем 1 и пишем в RTC
				else I2C_SendByte(RTC_ConvertFromDecToBD(7));					// Если нет, то пишем 7
			}
			break;
		case 5: // дата
			I2C_SendByte(4);             // Адрес даты 0x04
			date = RTC_ConvertFromBDToDec(date);  // Сперва приведем даты к нормальному значению, чтобы инкрементировать их
			if (direction)						  // Направление, в зависимости от того, какую кнопку нажимаем (прибавляем)
			{
				if (month == 2)  // если февраль, то...
				{
					if (year%4 == 0) // и если високосный год...
					{
						if(date < 29) I2C_SendByte(RTC_ConvertFromDecToBD(++date));	// Если < 29 то прибавляем 1 и пишем в RTC
						else I2C_SendByte(RTC_ConvertFromDecToBD(1));				// Если нет, то пишем 1
					} 
					else			// а если не високосный год...
					{
						if(date < 28) I2C_SendByte(RTC_ConvertFromDecToBD(++date));	// Если < 28 то прибавляем 1 и пишем в RTC
						else I2C_SendByte(RTC_ConvertFromDecToBD(1));				// Если нет, то пишем 1
					}
				}
				else if((month==4)|(month==6)|(month==9)|(month==11))  // Если у нас месяц, в котором 31 день
				{
					if(date < 30) I2C_SendByte(RTC_ConvertFromDecToBD(++date));		// Если < 30 то прибавляем 1 и пишем в RTC
					else I2C_SendByte(RTC_ConvertFromDecToBD(1));					// Если нет, то пишем 1
				}
				else                                                                 // Все остальные месяцы
				{
					if(date < 31) I2C_SendByte(RTC_ConvertFromDecToBD(++date));		// Если < 31 то прибавляем 1 и пишем в RTC
					else I2C_SendByte(RTC_ConvertFromDecToBD(1));					// Если нет, то пишем 1
				}
			}
			else						  // Направление, в зависимости от того (убавляем)
			{
				if (month == 2)  // если февраль, то...
				{
					if (year%4 == 0) // и если високосный год...
					{
						if(date > 1) I2C_SendByte(RTC_ConvertFromDecToBD(--date));	// Если > 1 то отнимаем 1 и пишем в RTC
						else I2C_SendByte(RTC_ConvertFromDecToBD(29));				// Если нет, то пишем 29
					} 
					else			// а если не високосный год...
					{
						if(date > 1) I2C_SendByte(RTC_ConvertFromDecToBD(--date));	// Если > 1 то отнимаем 1 и пишем в RTC
						else I2C_SendByte(RTC_ConvertFromDecToBD(28));				// Если нет, то пишем 28
					}
				}
				else if((month==4)|(month==6)|(month==9)|(month==11))  // Если у нас месяц, в котором 31 день
				{
					if(date > 1) I2C_SendByte(RTC_ConvertFromDecToBD(--date));	// Если > 1 то отнимаем 1 и пишем в RTC
					else I2C_SendByte(RTC_ConvertFromDecToBD(30));				// Если нет, то пишем 30
				}
				else                                                                 // Все остальные месяцы
				{
					if(date > 1) I2C_SendByte(RTC_ConvertFromDecToBD(--date));	// Если > 1 то отнимаем 1 и пишем в RTC
					else I2C_SendByte(RTC_ConvertFromDecToBD(31));				// Если нет, то пишем 31
				}
				
			}
			break;
		case 6: // месяц
			I2C_SendByte(5);             // Адрес месяца 0x05
			month = RTC_ConvertFromBDToDec(month);  // Сперва приведем месяц к нормальному значению, чтобы инкрементировать их
			if (direction)						  // Направление, в зависимости от того, какую кнопку нажимаем (прибавляем)
			{
				if(month < 12) I2C_SendByte(RTC_ConvertFromDecToBD(++month));		// Если < 12 то прибавляем 1 и пишем в RTC
				else I2C_SendByte(RTC_ConvertFromDecToBD(1));				// Если нет, то пишем 1
			}
			else						  // Направление, в зависимости от того, какую кнопку нажимаем (отнимаем)
			{
				if(month > 1) I2C_SendByte(RTC_ConvertFromDecToBD(--month));	// Если > 1 то отнимаем 1 и пишем в RTC
				else I2C_SendByte(RTC_ConvertFromDecToBD(12));				// Если нет, то пишем 12
			}
			break;
		case 7: // год
			I2C_SendByte(6);             // Адрес года 0x05
			year = RTC_ConvertFromBDToDec(year);  // Сперва приведем год к нормальному значению, чтобы инкрементировать их
			if (direction)						  // Направление, в зависимости от того, какую кнопку нажимаем (прибавляем)
			{
				if(year < 99) I2C_SendByte(RTC_ConvertFromDecToBD(++year));		// Если < 99 то прибавляем 1 и пишем в RTC
				else I2C_SendByte(RTC_ConvertFromDecToBD(1));				// Если нет, то пишем 1
			}
			else						  // Направление, в зависимости от того, какую кнопку нажимаем (отнимаем)
			{
				if(year > 1) I2C_SendByte(RTC_ConvertFromDecToBD(--year));	// Если > 1 то отнимаем 1 и пишем в RTC
				else I2C_SendByte(RTC_ConvertFromDecToBD(99));				// Если нет, то пишем 99
			}
			break;
	}
	I2C_StopCondition(); // Условие стоп
}
//===================================================================================================================
