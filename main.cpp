// Консольное приложение для ведения вишлиста (списка желаемых игр) видеоигр

/*/

Функционал вишлиста:

1. Посмотреть все игры в списке - за это отвечает функция ViewAllGames

2. Добавить игру - за это отвечает функция AddGame

3. Удалить игру - за это отвечает функция DeleteGame

4. Фильтр игр (фильтр по цене - FilterGame, году - FilterByYear и статусу - FilterByStatus)

5. Случайный выбор игры - за это отвечает функция  PickRandomGame

6. Сохранение в бинарный файл - за это отвечает функция SaveAndExit

7. Первоначальная регистрация пользователя - RegisterInteractiveUser, а также ввод логина или пароля при помощи пользователя - LoginInteractiveLoop

/*/

#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>
#include <cstring>
#include <windows.h> 
#include <vector> 
#include <sstream>
#include <ctime>

using namespace std;

enum PlatformFlags { 
    PF_PC = 1 << 0,
    PF_PlayStation = 1 << 1,
    PF_Xbox = 1 << 2,
    PF_NintendoSwitch = 1 << 3,
    PF_Other = 1 << 4
};

struct Game {
    int id;
    char title[21];
    int platform; 
    int year;
    double price;
    bool isCompleted;
};

// Функция для установки кодировки русской 
void SetConsoleEncoding() {
    SetConsoleCP(65001);
    SetConsoleOutputCP(65001);
}
//---------------------------------------------------------------------------------------------//
//  Доп. функция - читать все игры из бинарного файла 
vector<Game> ReadAllGamesFromFile(const string& filename = "igri.bin") {
    vector<Game> games;
    ifstream in(filename, ios::binary);
    if (!in) return games;
    Game tmp;
    while (in.read(reinterpret_cast<char*>(&tmp), sizeof(Game))) {
        games.push_back(tmp);
    }
    in.close();
    return games;
}
// Функция, при помощи которой может хранится несколько платформ сразу, а не 1
string PlatformMaskToString(int mask) {
    if (mask == 0) return "Unknown";
    string out;
    if (mask & PF_PC) out += "PC|";
    if (mask & PF_PlayStation) out += "PlayStation|";
    if (mask & PF_Xbox) out += "Xbox|";
    if (mask & PF_NintendoSwitch) out += "Switch|";
    if (mask & PF_Other) out += "Other|";
    if (!out.empty()) out.pop_back(); // убрать последний '|'
    return out;
}


//---------------------------------------------------------------------------------------------//
// Добавляет одну запись Game в конец бинарного файла (append)
void AddGame(const Game& g, const string& filename = "igri.bin") {
ofstream out(filename, ios::binary | ios::app);
    if (!out) {
cerr << "не удалось открыть файл для записи " << filename << '\n';
    return;
}
out.write(reinterpret_cast<const char*>(&g), sizeof(Game));
out.close();
cout << "Игра добавлена (id = " << g.id << ").\n";
}

// Генерация уникального айди для каждой игры
int generateNextId(const string& filename = "igri.bin") {
ifstream in(filename, ios::binary);
    if (!in) return 1; 

Game tmp;
int maxId = 0;
while (in.read(reinterpret_cast<char*>(&tmp), sizeof(Game))) {
    if (tmp.id > maxId) maxId = tmp.id;
}
    return maxId + 1;
}

Game inputNewGame(int id) {
Game g{};
g.id = id;

// Ввод названия
cout << "Введите название игры (от 1 до 20 символов!): ";
string line;
getline(cin, line);
auto lpos = line.find_first_not_of(" \t\r\n");
auto rpos = line.find_last_not_of(" \t\r\n");
    if (lpos == string::npos) line = "";
    else line = line.substr(lpos, rpos - lpos + 1);
    if (line.empty()) line = "Неизвестно";
    if (line.size() > 20) line = line.substr(0, 20);

strncpy(g.title, line.c_str(), 20);
g.title[20] = '\0';

// Ввод платформ
cout << "Платформа/ы, на которой выпускалась игра (0=PC,1=PlayStation,2=Xbox,3=Switch,4=Other).\n";
cout << "Можно ввести несколько через пробел или запятую, например: 0 2 или 0,2\n";
cout << "Ввод: ";
string platformsLine;
getline(cin, platformsLine);
    if (platformsLine.empty()) {
getline(cin, platformsLine);
}
    for (char &c : platformsLine) if (c == ',') c = ' ';
stringstream ss(platformsLine);
int p;
g.platform = 0;
bool any = false;
    while (ss >> p) {
    switch (p) {
    case 0: g.platform |= PF_PC; any = true; break;
    case 1: g.platform |= PF_PlayStation; any = true; break;
    case 2: g.platform |= PF_Xbox; any = true; break;
    case 3: g.platform |= PF_NintendoSwitch; any = true; break;
    case 4: g.platform |= PF_Other; any = true; break;
default:    
    cout << "Пропускаем неверный номер платформы: " << p << "\n";
    }
}
    if (!any) {
g.platform = PF_Other;
}

// Ввод года
cout << "Год выпуска: ";
    while (!(cin >> g.year) || g.year < 1950 || g.year > 3000) {
cin.clear();
cin.ignore(10000, '\n');
cout << "Неверно. Введите корректный год: ";
}

// Ввод цены
cout << "Цена в рублях: ";
    while (!(cin >> g.price) || g.price < 0.0) {
cin.clear();
cin.ignore(10000, '\n');
cout << "Неверно. Введите положительное число: ";
}

// Ввод статуса прохождения
cout << "Игра пройдена? (0 = нет, 1 = да): ";
int s;
    while (!(cin >> s) || (s != 0 && s != 1)) {
cin.clear();
cin.ignore(10000, '\n');
cout << "Неверно. Введите 0 или 1: ";
}
g.isCompleted = (s == 1);

cin.ignore(10000, '\n'); // очистка консоли от остатков строки
    return g;
}

//---------------------------------------------------------------------------------------------//
// Удаление игры по ID
void DeleteGame(int id, const string& filename = "igri.bin") {
ifstream in(filename, ios::binary);
    if (!in) {
cout << "Не удалось открыть файлик\n";
    return;
}

vector<Game> games;
Game tmp;
bool found = false;
    while (in.read(reinterpret_cast<char*>(&tmp), sizeof(Game))) {
    if (tmp.id != id) {
    games.push_back(tmp);
}
else {
    found = true; 
    }
}
in.close();

    if (!found) {
cout << "Игра с таким id не найдена\n";
    return;
}

ofstream out(filename, ios::binary | ios::trunc);
    for (const auto& game : games) {
out.write(reinterpret_cast<const char*>(&game), sizeof(Game));
}
out.close();
cout << "Игра с id " << id << " удалена.\n";
}

//---------------------------------------------------------------------------------------------//
//  Показать все игры в списке
void ViewAllGames(const string& filename = "igri.bin") {
ifstream in(filename, ios::binary);
    if (!in) {
cout << "Файл пуст или его вообще не создали\n";
    return;
}

Game tmp;
    while (in.read(reinterpret_cast<char*>(&tmp), sizeof(Game))) {
cout << "ID: " << tmp.id << ", Название: " << tmp.title << ", Платформа: " << PlatformMaskToString(tmp.platform) << ", Год: " << tmp.year << ", Цена: " << tmp.price << ", Пройдено или нет?: " << (tmp.isCompleted ? "Да" : "Нет") << endl;
    }
in.close();
}
//---------------------------------------------------------------------------------------------//
// Фильтр игр по цене (выводятся либо самые дешёвые, либо самые дорогие)

void FilterGame(const vector<Game>& games, bool cheapest = true) {
vector<Game> filteredGames;
    if (games.empty()) {
filteredGames = ReadAllGamesFromFile();
}
    else {
for (const auto& game : games) {
    filteredGames.push_back(game);
    }
}
    if (filteredGames.empty()) {
cout << "Нет игр для фильтрации.\n";
    return;
}

    if (cheapest) {
sort(filteredGames.begin(), filteredGames.end(), [](const Game& a, const Game& b) {
    return a.price < b.price;
});
}
    else {
sort(filteredGames.begin(), filteredGames.end(), [](const Game& a, const Game& b) {
    return a.price > b.price;
});
}
cout << "Отфильтрованные игры по цене:\n";
    for (const auto& game : filteredGames) {

cout << "ID: " << game.id << ", Название: " << game.title << ", Платформа: " << PlatformMaskToString(game.platform) << ", Год: " << game.year << ", Цена: " << game.price << ", Пройдено или нет: " << (game.isCompleted ? "Да" : "Нет") << endl;
    }
}

//---------------------------------------------------------------------------------------------//
// Фильтр по году (показывает игры старше/моложе порога)
void FilterByYear(const vector<Game>& games, int yearThreshold, bool showOlder) {
vector<Game> list = games.empty() ? ReadAllGamesFromFile() : games;
    if (list.empty()) { cout << "Нет данных для фильтрации по году!!\n"; return; }
cout << (showOlder ? "Старые игры (год <= " : "Новые игры (год >= ") << yearThreshold << "):\n";
    for (const auto& g : list) {
    if (showOlder) {
    if (g.year <= yearThreshold) {
cout << "ID: " << g.id << ", " << g.title << ", " << g.year << ", " << g.price << ", Пройдено: " << (g.isCompleted ? "Да" : "Нет") << '\n';
    }
}
    else {
    if (g.year >= yearThreshold) {
 cout << "ID: " << g.id << ", " << g.title << ", " << g.year << ", " << g.price << ", Пройдено: " << (g.isCompleted ? "Да" : "Нет") << '\n';
            }
        }
    }
}

//---------------------------------------------------------------------------------------------//
// Фильтр по статусу (пройдена/не пройдена)
void FilterByStatus(const vector<Game>& games, bool completed) {
vector<Game> list = games.empty() ? ReadAllGamesFromFile() : games;
    if (list.empty()) { cout << "Нет данных для фильтрации по статусу.\n"; return; }
cout << (completed ? "Пройденные игры:\n" : "Не пройденные игры:\n");
    for (const auto& g : list) {
    if (g.isCompleted == completed) {
cout << "ID: " << g.id << ", " << g.title << ", " << g.year << ", " << g.price << '\n';
        }
    }
}

//---------------------------------------------------------------------------------------------//
// Случайный выбор игры
void PickRandomGame(const vector<Game>& games) {
vector<Game> list = games.empty() ? ReadAllGamesFromFile() : games;
    if (list.empty()) {
cout << "Список игр пуст!!!\n";
    return;
}

srand(static_cast<unsigned int>(time(0))); //генератор случайных чисел
int randomIndex = rand() % list.size();  // здесь выбор случайного индекса
const Game& randomGame = list[randomIndex];
cout << "Случайно выбранная игра: " << randomGame.title << " (" << PlatformMaskToString(randomGame.platform) << ")" << endl;
}

//---------------------------------------------------------------------------------------------//
// Сохранение данных в файл
void SaveAndExit(const vector<Game>& games, const string& filename = "igri.bin") {
ofstream out(filename, ios::binary | ios::trunc);
    for (const auto& game : games) {
out.write(reinterpret_cast<const char*>(&game), sizeof(Game));
}
out.close();
cout << "Все изменения сохранены\n";
}



// Новая структура пользователь. Для регистрации и входа в аккаунт
//---------------------------------------------------------------------------------------------//
//---------------------------------------------------------------------------------------------//
struct User {
    int id;
    char username[21];
    char password[41]; 
};

User currentUserGlobal;
int currentUserId = -1;


void AddUser(const User& u, const string& filename = "login.bin") {
ofstream out(filename, ios::binary | ios::app);
    if (!out) {
cerr << "Не удалось открыть файл для записи " << filename << '\n';
    return;
}
out.write(reinterpret_cast<const char*>(&u), sizeof(User));
out.close();
}
//---------------------------------------------------------------------------------------------//
// Функциия, считывающая всех пользователей из файла
vector<User> ReadAllUsersFromFileUsers(const string& filename = "login.bin") {
vector<User> users;
ifstream in(filename, ios::binary);
    if (!in) return users;
User tmp;
    while (in.read(reinterpret_cast<char*>(&tmp), sizeof(User))) {
users.push_back(tmp);
}
in.close();
    return users;
}

//---------------------------------------------------------------------------------------------//
// Перезапись всех пользователей в файл
void SaveAllUsersToFile(const vector<User>& users, const string& filename = "login.bin") {
ofstream out(filename, ios::binary | ios::trunc);
    if (!out) {
cerr << "Не удалось открыть файл для записи " << filename << '\n';
    return;
}
    for (const auto& u : users) out.write(reinterpret_cast<const char*>(&u), sizeof(User));
out.close();
}

//---------------------------------------------------------------------------------------------//
// Генерация уникального id для пользователя
int generateNextUserId(const string& filename = "login.bin") {
ifstream in(filename, ios::binary);
    if (!in) return 1;
User tmp;
int maxId = 0;
    while (in.read(reinterpret_cast<char*>(&tmp), sizeof(User))) {
    if (tmp.id > maxId) maxId = tmp.id;
}
    return maxId + 1;
}

//---------------------------------------------------------------------------------------------//
// Проверка, существует ли имя пользователя
bool UsernameExists(const string& name, const string& filename = "login.bin") {
auto users = ReadAllUsersFromFileUsers(filename);
    for (const auto& u : users) {
    if (name == string(u.username)) return true;
    }
    return false;
}
//---------------------------------------------------------------------------------------------//

// Аутентификация
bool AuthenticateUser(const string& name, const string& password, User& outUser, const string& filename = "login.bin") {
auto users = ReadAllUsersFromFileUsers(filename);
    for (const auto& u : users) {
    if (name == string(u.username) && password == string(u.password)) {
outUser = u;
    return true;
    }
}
    return false;
}

//---------------------------------------------------------------------------------------------//
// Интерактивная регистрация (вставить вызов из EnsureAuthenticated при первом запуске)
void RegisterInteractiveUser(const string& filename = "login.bin") {
cout << "\n-+-+-+-+-+-+Регистрация пользователя-+-+-+-+-+-+\n";
string name;
    while (true) {
cout << "Введите ваш логин (1-20 символов): ";
getline(cin, name);
auto lpos = name.find_first_not_of(" \t\r\n");
auto rpos = name.find_last_not_of(" \t\r\n");
    if (lpos == string::npos) name = "";
    else name = name.substr(lpos, rpos - lpos + 1);
    if (name.empty()) { cout << "Логин не может быть пустым!!\n"; continue; }
    if (name.size() > 20) name = name.substr(0, 20);
    if (UsernameExists(name, filename)) {
cout << "Пользователь с таким логином уже существует. Введите другой логин\n";
    continue;
}
    break;
}

string pass;
    while (true) {
cout << "Введите пароль (1-40 символов): ";
getline(cin, pass);
    if (pass.empty()) { cout << "Пароль не может быть пустым!!!!!\n"; continue; }
    if (pass.size() > 40) pass = pass.substr(0, 40);
    break;
}

User u{};
    u.id = generateNextUserId(filename);
    strncpy(u.username, name.c_str(), 20);
    u.username[20] = '\0';
    strncpy(u.password, pass.c_str(), 40);
    u.password[40] = '\0';

AddUser(u, filename);
cout << "Регистрация завершена. Ваш ID = " << u.id << ".\n";
currentUserGlobal = u;
currentUserId = u.id;
}

//---------------------------------------------------------------------------------------------//
// Ввод логина/пароля и попытка аутентификации 
void LoginInteractiveLoop(const string& filename = "login.bin") {
cout << "\n-+-+-+-+-+-+Вход в систему-+-+-+-+-+-+\n";
    while (true) {
string name, pass;
cout << "Логин: ";
getline(cin, name);
cout << "Пароль: ";
getline(cin, pass);
User u{};
    if (AuthenticateUser(name, pass, u, filename)) {
currentUserGlobal = u;
currentUserId = u.id;
cout << "Успешный вход. Привет пользователь " << string(u.username) << "!\n";
    break;
}
    else {
cout << "Неверный логин или пароль, попробуйте ещё раз\n";
    }
    }
}

//---------------------------------------------------------------------------------------------//
// Проверка: есть ли в системе хотя бы один пользователь
bool HasAnyUser(const string& filename = "login.bin") {
ifstream in(filename, ios::binary);
    if (!in) return false;
User tmp;
bool any = false;
    if (in.read(reinterpret_cast<char*>(&tmp), sizeof(User))) any = true;
in.close();
    return any;
}

//---------------------------------------------------------------------------------------------//
//  Функция, управлящая первым экраном регистрации/входа.

void EnsureAuthenticated(const string& filename = "login.bin") {
currentUserId = -1;
memset(&currentUserGlobal, 0, sizeof(currentUserGlobal));

bool exists = HasAnyUser(filename);
    if (!exists) {
cout << "Сначала нужно зарегистрироваться!\n";
RegisterInteractiveUser(filename);
}
    else {
LoginInteractiveLoop(filename);
    }
}

//---------------------------------------------------------------------------------------------//
// Функция логирования действий пользователя 
void LogActionUser(int userId, const string& action, int gameId = -1, const string& filename = "deystviya.log") {
ofstream out(filename, ios::app);
    if (!out) return;
time_t t = time(nullptr);
out << t << " | user:" << userId << " | game:" << gameId << " | " << action << '\n';
out.close();
}



int main() {
// для вывода кириллицы
    SetConsoleEncoding();
    EnsureAuthenticated();

    vector<Game> games;
    games = ReadAllGamesFromFile();

int choice;
bool changesSaved = false; // Специальный флаг для отслеживания, сохранялись ли изменения
    while (true) {
cout << "\n~=~=~=~=~=~=~=КОНСОЛЬНАЯ МЕНЮШКА~=~=~=~=~=~=~=:\n";
cout << "1. Посмотреть все игры\n";
cout << "2. Добавить игру\n";
cout << "3. Удалить игру\n";
cout << "4. Фильтровать игры по разным критериям\n";
cout << "5. Случайный выбор игры\n";
cout << "6. Сохранить и выйти\n";
cout << "Выберите нужную функцию: ";
cin >> choice;
cin.ignore();

    if (choice == 1) {
ViewAllGames();
}
    else if (choice == 2) {
int id = generateNextId();

Game g = inputNewGame(id);

AddGame(g);
games.push_back(g);
changesSaved = false;
}
    else if (choice == 3) {

int id;
cout << "Введите ID игры для удаления: ";
cin >> id;
DeleteGame(id);
games = ReadAllGamesFromFile(); 
changesSaved = false;
changesSaved = false;
cin.ignore(10000, '\n');
}
    else if (choice == 4) {

    while (true) {
cout << "\n~=~=~=~=~=~=~=Меню фильтровв~=~=~=~=~=~=~=\n";
cout << "1. Фильтр по цене (самые дешёвые или же самые дорогие)\n";
cout << "2. Фильтр по году (старые или новые)\n";
cout << "3. Фильтр по статусу (пройдена или не пройдена)\n";
cout << "4. Выйти из меню фильтров\n";
cout << "Выберите фильтр: ";
int fchoice;
    if (!(cin >> fchoice)) { cin.clear(); cin.ignore(10000, '\n'); cout << "Неверный ввод\n"; continue; }
cin.ignore(10000, '\n');

    if (fchoice == 1) {
int priceChoice;
cout << "Фильтр по самым дешевым играм (0) или самым дорогим (1) играм? ";
cin >> priceChoice;
cin.ignore(10000, '\n');
FilterGame(games, priceChoice == 0);
}
    else if (fchoice == 2) {
int mode;
cout << "Показать старые (0) или новые (1) игры? ";
cin >> mode;
cout << "Введите пороговый год (например 2015): ";
int threshold;
cin >> threshold;
cin.ignore(10000, '\n');
FilterByYear(games, threshold, mode == 0);
}
    else if (fchoice == 3) {
int st;
cout << "Показать не пройденные (0) или пройденные (1) игры? ";
cin >> st;
cin.ignore(10000, '\n');
FilterByStatus(games, st == 1);
}
    else if (fchoice == 4) {

    break;
}
    else {
cout << "Неверный выбор в меню фильтров.\n";
        }
    }
}
    else if (choice == 5) {
PickRandomGame(games);
}
    else if (choice == 6) {
    if (!changesSaved) {
cout << "Сохранить изменения в файл?? (1 - ага, 0 - неее): ";
int saveChoice;
cin >> saveChoice;
if (saveChoice == 1) {
SaveAndExit(games);
}
    else {
cout << "Изменения не сохранены :(\n";
    }
}
    else {
cout << "Все изменения сохранены ураа\n";
}
    break;
}
    else {
cout << "Неверный выбор. Попробуйте снова.\n";
    }
}

    return 0;
}
