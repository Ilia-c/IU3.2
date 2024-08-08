typedef struct  MAKE_MENU{
    const char  Name[20];        // Название пункта меню
    const char  Fonts[10];        // Шрифт пункта
    int         Type_menu;       // тип меню - вкладка(0)/значение int(1)/значение float(2)/действие(3). На действие проверяется параметр Next.

    void        *Next;           // Следующий пункт меню (Id)
	void        *Previous;       // Предыдущий пункт меню (Id)

	void        *Parent;         // Родительский пункт меню  
	void        *Child;          // На какой пункт меню ссылается 

    int         *Data_int;       // данные типа int для редактирования
    float       *Data_float;     // данные типа float для редактирования
} menuItem;