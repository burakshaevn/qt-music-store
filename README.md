# Десктоп приложение «Магазин музыкальных инструментов»

## Описание
Разработано с помощью C++20 с использованием фреймворка Qt 6.8.1 и системой управления базами данных PostgreSQL. Проект представляет собой десктоп приложение для управления данными музыкального магазина, включая: клиентов, администраторов, музыкальные инструменты, подборки музыкальных инструментов и управление покупками. Данные извлекаются из базы данных, локально кэшируются и отображаются в интерфейсе с использованием Qt. 

В программе предусмотрено два пользователя — **клиент** и **администратор**.
* Администратор имеет возможность добавлять, редактировать и удалять данные. К
* Клиент имеет возможность лишь просматривать подборки, совершать покупки и имеет личный кабинет.

## Сборка и установка
Требования:
- C++20
- Qt 6.8.1+
- PostgreSQL 17+
- CMake 3.5+

Сборка и запуск:

В переменных средах системы должен (PATH) быть указан путь до бинарных библиотек Postgres.
* Windows (_пример_): `C:\dev\PostgreSQL\17\lib`
* MacOS (_пример_): `/Library/PostgreSQL/17/lib`

### Создание базы данных
* В репозитории хранится файл c расширением `.sql`, содержащий пример базы данных для задачи.

  ```
  1. Выполняем авторизацию в pgAdmin 4.
  2. Создаём базу данных, в которой будут расположены таблицы, триггеры и тд.
  3. Открываем «запросник» Query Tool → Вставить код из файла .sql → Выполнить запрос.
  ```

### Сборка под редакторы

* Qt Creator
  ```
  1. Распаковать репозиторий.
  2. Open Qt Creator → Open Project.
  3. В открывшемся окне указываем путь к распакованному репозиторию и выбираем файл CMakeLists.txt.
  4. Выполняем конфигурацию проекта под нужный компилятор «Configure Project».
  ```

* Visual Studio 2022
  ```sh
  cd ..\qt-transport-catalogue\
  mkdir build-vs
  cd ..\qt-transport-catalogue\build-vs
  cmake .. -G "Visual Studio 17 2022" -DCMAKE_PREFIX_PATH="C:\Qt\6.8.1\msvc2022_64\"
  mingw32-make
  ```
  где `-DCMAKE_PREFIX_PATH="C:\Qt\6.8.1\msvc2022_64\"` — путь к месту установки фреймворка Qt, собранного с использованием компилятора Microsoft Visual Studio. Здесь можно указать свой компилятор.

## Пример  
<div align="center">
  <img src="https://github.com/user-attachments/assets/67ac5082-15e1-49f1-b2e7-cf5cd1834e7b" alt="image" width = "70%">
  <p>Рис. 1 — Главный экран.</p>
</div> 

## Авторизация
В программе предусмотрено два пользователя — **клиент** и **администратор**.  

<div align="center">
  <img src="https://github.com/user-attachments/assets/536e6af2-cb38-40f6-b976-99d67e1969cf" alt="image" width = "70%">
  <p>Рис. 2 — Окно авторизации.</p>
</div> 

<details>
<summary>Авторизация как администратор</summary>
  
  Требуется таблица admins.
  1. Открыть pgAdmin 4
  2. Открыть Query Tool для базы данных
  3. Выполнить SQL-запрос:
  ```sql
  SELECT * FROM admins;
  ```
  4. Скопировать любой email и вставить в поле Login
  5. Скопировать пароль выбранного почтового адреса и вставить в поле Password

</details>

<details>
<summary>Авторизация как клиент</summary>

  Требуется таблица clients.
  1. Открыть pgAdmin 4
  2. Открыть Query Tool для базы данных
  3. Выполнить SQL-запрос:
  ```sql
  SELECT * FROM clients;
  ```
  5. Скопировать любой email и вставить в поле Login
  6. Скопировать пароль выбранного почтового адреса и вставить в поле Password

</details>

## Возможности клиента
После авторизации справа появляется плавающее меню. Доступно 4 кнопки — _главная, поиск, корзина и профиль_. 

### Главна страница
Сюда пользователь попадает после нажатия на самую верхнюю кнопку в плавающем меню (три точки). В боковом меню доступны категории товаров. Если нажать «Смотреть всё», на экране будут отображаться все доступные товары в магазине (пример см. ниже).
<div align="center">
  <img src="https://github.com/user-attachments/assets/15835cfc-ff7f-4dc2-b57a-21f4242b868d" alt="GIF Image" width="50%">
  <p>Рис. 3 — Главная страница после авторизации как пользователь.</p>
</div>  

Добавление/Удаление в корзину происходит с помощью кнопок на карточке товара в правом верхнем углу. 
<div align="center">
  <img src="https://github.com/user-attachments/assets/05810f37-b5e9-42e3-9d3f-95a607eb8d8a" alt="GIF Image" width="50%">
  <p>Рис. 4 — Товар не добавлен в корзину.</p>
</div> 
<div align="center">
  <img src="https://github.com/user-attachments/assets/3be35b1a-7f4f-41c7-b937-af1f0d9f935f" alt="GIF Image" width="50%">
  <p>Рис. 5 — Товар добавлен в корзину.</p>
</div>   

### Поиск
После нажатия на кнопку поиска, выходит окно с полем для ввода. 
<div align="center">
  <img src="https://github.com/user-attachments/assets/7c89fc61-e8c1-4201-9d9c-0ff8372b2638" alt="GIF Image">
  <p>Рис. 6 — Ожидание поискового запроса.</p>
</div> 

Из базы данных происходит загрузка всех инструментов в локальный кэш — класс Instruments, а именно `std::unordered_map<QString, InstrumentInfo, InstrumentsHasher>`, где `QString` — название инструмента, `InstrumentInfo` — структура, хранит полную информацию об инструменте, `InstrumentsHashed` — хэшер для хэш-таблицы. В `unordered_map` хранятся только структуры самих инструментов. Так как это неупорядоченный словарь, то при каждом нажатии на «Смотреть всё» карточки будут каждый раз в разном порядке. 

Сами карточки кэшируются в классе MainWindow, в контейнере `std::unordered_map<QString, QWidget*> instruments_cards_;`, где `QString` — название инструмента, `QWidget*` указатель на полностью собранную карточку этого инструмента.
  
Данные кэшируются для быстрой отрисовки на нужной странице и выполнение быстрого поиска и последующее отображение результатов на экране. Если отказаться от кэширования и выполнять запросы напрямую в базу данных для отрисовки каждой карточки, будет утрачена скорость работы программы и придётся ждать достаточное время даже при переключении на любую вкладку. 

После выполнения поиска (кнопка «ОК») выполняется ранжирование наиболее релевантных запросов в кэше с помощью алгоритма TF-IDF: он оценивает важность слова в документе относительно всего корпуса текстов, основываясь на частоте термина в документе и его редкости в общем наборе документов. 
<div align="center">
  <img src="https://github.com/user-attachments/assets/9dce4383-021a-4c45-aa06-518399775e51" alt="GIF Image">
  <p>Рис. 7 — Уведомление о результатах поиска.</p>
</div> 

Отображение результатов поиска.
<div align="center">
  <img src="https://github.com/user-attachments/assets/8378f18a-ebff-4369-8392-98488fb225e4" alt="GIF Image" width = "70%">
  <p>Рис. 8 — Вывод результатов поиска.</p>
</div> 

### Корзина
В корзину добавляются все карточки товаров, которые были отмечены закладкой в правом верхнем углу.

На самой странице корзины:
* сверху отображается общая сумма корзины,
* `2 шт.` — количество предметов, добавленных в корзину,
* самая верхняя кнопка очищает содержимое корзины,
* самая нижняя кнопка выполняет оплату.

<div align="center">
  <img src="https://github.com/user-attachments/assets/317d15b8-0966-4068-be3e-a5a4e6fc2923" alt="Image" width="70%">
  <p>Рис. 9 — Страница корзины.</p>
</div>  

Выполняется запрос `INSERT`в базу данных:
```sql
INSERT INTO public.purchases(id, client_id, instrument_id) VALUES (%1, %2, %3);
```

где `%1` — порядковый номер новой записи в таблице `purchases` (он подбирается автоматически), `%2` — id клиента (ссылка на столбец id из таблицы clients), `%3` — id купленного инструмента (ссылка на столбец id из таблицы instruments).

Купленный инструмент отображается в таблице `purchases` и отображается в профиле.
<div align="center">
  <img src="https://github.com/user-attachments/assets/6a3e564c-c907-4331-9973-a8e65fa5658a" alt="Image" width="40%">
  <p>Рис. 8 — Подтверждение операции.</p>
</div> 
  
### Профиль
Здесь отображается виджет, который можно прокручивать вверх-вниз, который содержит в себе карточки купленных товаров клиентом. Сверху присутствует кнопка «**>**» отвечающая за выход из профиля. 
<div align="center">
  <img src="https://github.com/user-attachments/assets/9349e67d-05a9-402c-92bf-91156e61b15a" alt="Image" width="70%">
  <p>Рис. 9 — Страница профиля.</p>
</div> 
  
## Возможности администратора
По умолчанию эта страница будет пустой до тех пор, пока не будет выбрана таблица для редактирования. Чтобы это сделать, необходимо нажать на выпадающий список в верхней части экрана. Это виджет, в который динамически добавятся все таблицы из базы данных. 
<div align="center">
  <img src="https://github.com/user-attachments/assets/1e78de30-5cbb-4211-b5ac-7eea98c30982" alt="image" width="50%">
  <p>Рис. 10 — Страница по умолчанию для администратора.</p>
</div>

После выбора таблицы её данные отображаются на экране.
<div align="center">
  <img src="https://github.com/user-attachments/assets/bb6c4217-072d-4475-a570-e82f7a09605a" alt="image" width="50%">
  <p>Рис. 11 — Выгрузка данных из таблицы.</p>
</div>

Если плавающее меню администратора перекрывает часть данных, есть возможность его перетаскивать в пределах экрана. Логику реализует сигнал `bool Table::eventFilter(QObject* obj, QEvent* event)`.
<div align="center">
  <img src="https://github.com/user-attachments/assets/4a039503-255c-492b-be0c-be83d37ccd1a" alt="image" width="50%">
  <p>Рис. 12 — Динамическое изменение позиции плавающего меню.</p>
</div>

### Добавление записи
После нажатия на «Добавить», происходит вызов функции `void Table::AddRecord();`, которая в себе создаёт экземпляр класса EditDialog, в котором строится диалоговое окно `EditDialog dialog(newRecord, this);`, где `newRecord` — передача конкретной записи из базы данных в виде QSqlRecord. Эта запись используется для отображения столбцов таблицы.

Здесь нельзя задать значение поля id. Потому что это «счётчик» записей в таблице, который будет сам автоматически увеличиваться по мере поступления новых записей.
<div align="center">
  <img src="https://github.com/user-attachments/assets/99fa8964-ae2c-4afa-b854-ef86a75a7506" alt="image">
  <p>Рис. 13 — Окно для добавления.</p>
</div>

## Удаление записи
Производится ввод ID записи в таблице. Она будет удалена.

В некоторых таблицах, например, ID начинается необязательно с 1. Итерироваться в этом окне мы можем от самого минимального ID до самого максимального. Чтобы не выходить за пределы.
<div align="center">
  <img src="https://github.com/user-attachments/assets/ea9cfb6a-0ab8-4f15-a25f-dcd3543fb627" alt="image">
  <p>Рис. 14 — Окно ввода ID записи (автоинкрементируемого столбца) в текущей таблице.</p>
</div>

После указания ID удаляемой записи выходит окно подтверждения удаления, где строится таблица с удаляемой строкой (чтобы быть уверенным, что удаляется именно то, что мы задумали).
<div align="center">
  <img src="https://github.com/user-attachments/assets/ca9aa27c-eaf9-4f1a-bfbe-325f200deb52" alt="image">
  <p>Рис. 15 — Вывод удаляемой строки.</p>
</div>

После подтверждения происходит удаление из основной таблицы, **а записи, которые ссылались на первичный ключ этой строки, каскадно удаляются**.
<div align="center">
  <img src="https://github.com/user-attachments/assets/6a2f0e82-9ee6-45f0-84bb-a17c302c5835" alt="image">
  <p>Рис. 16 — Окно подтверждения удаления.</p>
</div>

## Редактирование записи
Для редактирования указывается всегда номер строки в таблицы, а не ID записи. 
<div align="center">
  <img src="https://github.com/user-attachments/assets/0bd0d248-54ce-4bb0-b2e4-b166a7e5fbe3" alt="image">
  <p>Рис. 17 — Окно поиска записи в таблице.</p>
</div>

Здесь редактирование порядкового номера записи (id) также недоступно.
<div align="center">
  <img src="https://github.com/user-attachments/assets/04cba3de-bf20-44a3-8ab7-13dac0666ff6" alt="image">
  <p>Рис. 18 — Окно редактирования записи.</p>
</div>

## ER-диаграмма базы данных
<div align="center">
  <img src="https://github.com/user-attachments/assets/0d8d221d-a6ec-4910-a8a6-bd1f7ef6f63c" alt="image">
  <p>Рис. 19 — ER-диаграмма.</p>
</div>  
