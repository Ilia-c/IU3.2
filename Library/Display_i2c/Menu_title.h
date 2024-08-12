typedef struct  MAKE_MENU{
    const char  Name_rus[20];    // Название пункта меню на русском
    const char  Name_en[20];     // Название пункта меню на английском
    const char  Fonts[10];       // Шрифт пункта
    int         Type_menu;       // тип меню - вкладка(0)/значение ввод int(1)/значение ввод float(2)/значение вывод - float(требуется преобразование) (3)/действие(4)/инженерное меню(5). На действие проверяется параметр Next.

    void        *Next;           // Следующий пункт меню (Id)
	void        *Previous;       // Предыдущий пункт меню (Id)

	void        *Parent;         // Родительский пункт меню  
	void        *Child;          // На какой пункт меню ссылается 

    float       *data_out;
    int         *Data_int;       // данные типа int для редактирования
    float       *Data_float;     // данные типа float для редактирования
} menuItem;