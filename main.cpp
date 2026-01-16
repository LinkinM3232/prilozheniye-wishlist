// Консольное приложение для ведения вишлиста (списка желаемых игр) видеоигр

/*/

Функционал вишлиста:

1. Посмотреть все игры в списке - за это отвечает функция ViewAllGames

2. Добавить игру - за это отвечает функция AddGame

3. Удалить игру - за это отвечает функция DeleteGame

4. Фильтр игр (фильтр по цене - FilterGame, году - FilterByYear и статусу - FilterByStatus)

5. Случайный выбор игры - за это отвечает функция  PickRandomGame

6. Сохранение в бинарный файл - за это отвечает функция SaveAndExit

/*/

#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>
#include <cstring>
#include <windows.h> 
#include <vector> 
#include <sstream>

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

