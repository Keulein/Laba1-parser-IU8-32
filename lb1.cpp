
#include <iostream>
#include <map>
#include <list>
#include <set>
#include <vector>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <locale>

using namespace std;

// замена в строке одного текста на другой
void replace_all(string& str, const string& from, const string& to) {
	size_t start_pos = 0;
	while ((start_pos = str.find(from, start_pos)) != string::npos) {
		str.replace(start_pos, from.length(), to);
		start_pos += to.length();
	}
}

// делит строку по разделителям, возвращает вектор из строк
vector<string> split(const string& s, char delim) {
	stringstream ss(s);
	string item;
	vector<string> elems;

	while (getline(ss, item, delim)) {
		elems.push_back(move(item)); 
	}
	return elems;
}

// функции, чтобы убрать лишние пробелы в начале и конце строки
static inline void ltrim(std::string& s) {
	s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int ch) {
		return !std::isspace(ch);
		}));
}

// trim from end (in place)
static inline void rtrim(std::string& s) {
	s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch) {
		return !std::isspace(ch);
		}).base(), s.end());
}

// trim from both ends (in place)
static inline void trim(std::string& s) {
	ltrim(s);
	rtrim(s);
}

// trim from start (copying)
static inline std::string ltrim_copy(std::string s) {
	ltrim(s);
	return s;
}

// trim from end (copying)
static inline std::string rtrim_copy(std::string s) {
	rtrim(s);
	return s;
}

// trim from both ends (copying)
static inline std::string trim_copy(std::string s) {
	trim(s);
	return s;
}

// возвращает строку без кавычек
string remove_quotes(string& in_string) {
	string s = trim_copy(in_string);
	return trim_copy(s).substr(1, s.length() - 2);
}

// базовый объект для хранения значений
struct MyJsonObject
{
	int    type_of_value = 0;   // здесь будет хранится тип значения

	string unprocessed = "";    // 0 значение "как есть", которое мы получили при разделении строки

	int    value_int = 0;       // 1
	double value_double = 0;    // 2
	string value_str = "";      // 3
	bool   value_bool = false;  // 4
	bool   value_null = false;  // 5 - элемент содержит "пустое значение"

	string link_to = "";        // 6 - элемент ссылается на другой элемент
	list<MyJsonObject>         my_list;   // 7 - элемент содержит список значений  []
	map<string, MyJsonObject>  my_map;    // 8 - элемент содержит словарь значений {}

};


string extract_substring(string& in_string, string& beg_symbol, string& end_symbol) {
	size_t beg_pos;
	size_t end_pos = in_string.find(end_symbol);

	if (end_pos != string::npos) {      
		beg_pos = in_string.rfind(beg_symbol, end_pos);
		if (end_pos != string::npos) {  // нашли открывающую скобку, ближайшую к закрывющей
			return in_string.substr(beg_pos, end_pos - beg_pos + 1);
		}
		else {  
			return "";
		}

	}
	else {
		return "";
	}
}


// если нам передан список, а не ассоциативный массив - делаем из списка ассоциативный массив
void pravilnaya_skobka(string& in_string) {
	size_t beg_pos1 = in_string.find("{");
	size_t beg_pos2 = in_string.find("[");

	if (beg_pos2 != string::npos and beg_pos2 < beg_pos1) {  // нам передан список, а не словарь
		in_string = "{\"pustoy_kluch\": " + in_string + "}";
	}
}


// проходим по строке и в цикле заменяем всё что можно на указатель
void obrabotat_skobki(map<string, MyJsonObject>& slovar, string& in_string, string& skobka1_beg, string& skobka1_end) {
	size_t prev_str_length;
	string key_patt;                   // ключ для нашего фрагмента строки
	int max_iter = in_string.length(); 
	int iter = 0;                      


	do {
		prev_str_length = in_string.length();
		
		cout << endl << "Итерация № " << iter + 1 << endl;
		cout << "Строка до замены ==>" << in_string << endl;

		// фрагмент строки, который не содержит других скобок (только в началае и конце строки)
		string substr = extract_substring(in_string, skobka1_beg, skobka1_end);
		// но в этой строке ещё могут быть скобки другого вида

		if (substr.length() > 0) {

			// проверяем, что в значениях ассоциативного массива ещё нет такого же значения
			bool chk = false;
			for (map<string, MyJsonObject>::iterator it = slovar.begin(); it != slovar.end(); ++it) {
				if (substr == it->second.unprocessed) {
					chk = true;
				};
			}
			if (!chk) {  // если значения нет, то добавляем
				key_patt = "@#" + to_string(slovar.size() + 1) + "#@";
				MyJsonObject obj;  // задаём тип поля и значение поля
				obj.type_of_value = 0;
				obj.unprocessed = substr;
				slovar.insert(pair <string, MyJsonObject>(key_patt, obj));

				replace_all(in_string, substr, key_patt);
			}
			cout << "Строка после замены ==>" << in_string << endl;
		}
		else {
			cout << "Больше нет таких скобок" << endl;
		}

		iter++;
	} while (iter < max_iter and prev_str_length != in_string.length());
}



void print_slovar(map<string, MyJsonObject>& slovar) {
	cout << endl << endl << "-----------------------------" << endl;
	for (map<string, MyJsonObject>::iterator it = slovar.begin(); it != slovar.end(); ++it) {
		cout << it->first << " => " << it->second.unprocessed << '\n';
	}
	cout << "-----------------------------" << endl;
}


// печатает на экране наш словарь (выводит значения в объектах)
void print_slovar2_map(int my_type, map<string, MyJsonObject>& my_map, list<MyJsonObject>& my_list, int otstup) {
	map<string, MyJsonObject> empty_map;  // объекты-заглушки для вызова функции
	list<MyJsonObject> empty_list;

	string otsup_str = string(otstup * 2, ' ');  // задаём отступ для текста

	if (my_type == 0) {
		cout << otsup_str << "{" << endl;
		for (map<string, MyJsonObject>::iterator it = my_map.begin(); it != my_map.end(); ++it) {
			if (it->second.type_of_value == 7) {
				print_slovar2_map(1, empty_map, it->second.my_list, otstup + 1);
			}
			else if (it->second.type_of_value == 8) {
				print_slovar2_map(0, it->second.my_map, empty_list, otstup + 1);
			}
			else {

				if (it->second.type_of_value == 1) {
					cout << otsup_str << "  " << it->first << ": int(" << it->second.value_int << ")" << endl;
				}
				else if (it->second.type_of_value == 2) {
					cout << otsup_str << "  " << it->first << ": dbl(" << it->second.value_double << ")" << endl;
				}
				else if (it->second.type_of_value == 3) {
					cout << otsup_str << "  " << it->first << ": str(" << it->second.value_str << ")" << endl;
				}
				else if (it->second.type_of_value == 4) {
					cout << otsup_str << "  " << it->first << ": bool(" << it->second.value_bool << ")" << endl;
				}
				else if (it->second.type_of_value == 5) {
					cout << otsup_str << "  " << it->first << ": null(" << it->second.value_null << ")" << endl;
				}
			}
		}
		cout << otsup_str << "}" << endl;

	}
	else if (my_type == 1) {
		cout << otsup_str << "[" << endl;
		for (list<MyJsonObject>::iterator it = my_list.begin(); it != my_list.end(); ++it) {
			if (it->type_of_value == 7) {
				print_slovar2_map(1, empty_map, it->my_list, otstup + 1);
			}
			else if (it->type_of_value == 8) {
				print_slovar2_map(0, it->my_map, empty_list, otstup + 1);
			}
			else {
				string otsup_str = string(otstup * 4, ' ');  //
				if (it->type_of_value == 1) {
					cout << otsup_str << "  int(" << it->value_int << ")" << endl;
				}
				else if (it->type_of_value == 2) {
					cout << otsup_str << "  dbl(" << it->value_double << ")" << endl;
				}
				else if (it->type_of_value == 3) {
					cout << otsup_str << "  str(" << it->value_str << ")" << endl;
				}
				else if (it->type_of_value == 4) {
					cout << otsup_str << "  bool(" << it->value_bool << ")" << endl;
				}
				else if (it->type_of_value == 5) {
					cout << otsup_str << "  null(" << it->value_null << ")" << endl;
				}
			}
		}
		cout << otsup_str << "]" << endl;
	}
}


// печатает на экране наш словарь
void print_slovar2(map<string, MyJsonObject>& slovar) {
	cout << endl << endl << "-----------------------------" << endl;
	// выводим заполненный ассоциативный массив на экран
	for (map<string, MyJsonObject>::iterator it = slovar.begin(); it != slovar.end(); ++it) {
		cout << it->first << " => (" << it->second.type_of_value << ") :" << it->second.unprocessed << '\n';
	}
	cout << "-----------------------------" << endl;
}

// преобразует текст в объект вида MyJsonObject
MyJsonObject translate_string_to_obj(string in_string) {

	MyJsonObject out_elem;

	bool chk1 = (in_string.substr(0, 1) == "\"");  // начинается с двойной кавычки
	bool chk2 = (in_string == "true");             // true
	bool chk3 = (in_string == "false");            // false
	bool chk4 = (in_string == "null");             // null
	size_t start_pos;
	bool chk5 = ((start_pos = in_string.find(".")) != string::npos);  // содержит десятичную точку
	size_t start_pos_link, end_pos_link;
	bool chk6 = ((start_pos_link = in_string.find("@#")) != string::npos);  // содержит начало указателя
	bool chk7 = ((end_pos_link = in_string.find("#@")) != string::npos);  // содержит конец указателя


	if (chk1) {                 // если начинается с кавычки - значит текст
		out_elem.type_of_value = 3;
		out_elem.value_str = remove_quotes(in_string);  // текст без кавычек

	}
	else if (chk2 or chk3) {  // true или false значение
		out_elem.type_of_value = 4;
		out_elem.value_bool = chk2;

	}
	else if (chk4) {          // null (пустое значение)
		out_elem.type_of_value = 5;
		out_elem.value_null = chk4;

	}
	else if (chk5) {          // дробное число
		out_elem.type_of_value = 2;
		out_elem.value_double = stod(in_string);

	}
	else if (chk6 and chk7) { // ссылка
		out_elem.type_of_value = 6;
		out_elem.link_to = trim_copy(in_string);

	}
	else {                    // целое число
		out_elem.type_of_value = 1;
		out_elem.value_int = stoi(in_string);
	}

	return out_elem;
}


// собираем дерево из кусочков
// рекурсивный обход списка, вместо ссылок подтягиваем списки или словари
void process_map(map<string, MyJsonObject>& slovar1, string element_key) {
	if (slovar1[element_key].type_of_value == 8) {                       // если мы внутри словаря
		map<string, MyJsonObject>& my_map = slovar1[element_key].my_map;  // получаем ссылку на словарь

		for (map<string, MyJsonObject>::iterator it = my_map.begin(); it != my_map.end(); ++it) {  // обходим элементы словаря
			if (it->second.type_of_value == 6) {  // если нашли ссылку

				MyJsonObject& linked_element = slovar1[it->second.link_to];  // получили объект, на который указывает ссылка
				if (linked_element.type_of_value == 7) {         // если это список
					it->second.type_of_value = 7;                // меняем тип элемента со ссылки на список
					process_map(slovar1, it->second.link_to);    // запускаем рекурсивный обход по подключенному списку
					it->second.my_list = slovar1[it->second.link_to].my_list; // подключаем найденный список к текущему узлу
					it->second.link_to = "";                     // очистили ссылку

				}
				else if (linked_element.type_of_value == 8) {  // если это словарь
					it->second.type_of_value = 8;                // меняем тип элемента со ссылки на словарь
					process_map(slovar1, it->second.link_to);    // запускаем рекурсивный обход по подключенному словарю
					it->second.my_map = slovar1[it->second.link_to].my_map;   // подключаем найденный словарь к текущему узлу
					it->second.link_to = "";                     // очистили ссылку
				}
			}
		}

	}
	else if (slovar1[element_key].type_of_value == 7) {           // если мы внутри списка
		list<MyJsonObject>& my_list = slovar1[element_key].my_list;  // получаем ссылку на список

		for (list<MyJsonObject>::iterator it = my_list.begin(); it != my_list.end(); ++it) {  // обходим элементы списка
			// дальше код аналогичен, только итератор другого типа (цикл по list, а не по map)
			if (it->type_of_value == 6) {
				MyJsonObject& linked_element = slovar1[it->link_to];
				if (linked_element.type_of_value == 7) {
					it->type_of_value = 7;
					process_map(slovar1, it->link_to);
					it->my_list = slovar1[it->link_to].my_list;
					it->link_to = "";

				}
				else if (linked_element.type_of_value == 8) {
					it->type_of_value = 8;
					process_map(slovar1, it->link_to);
					it->my_map = slovar1[it->link_to].my_map;
					it->link_to = "";
				}
			}
		}
	}
}



// самая главная процедура
void parse_json(string in_string, MyJsonObject& result_obj) {
	string print_str;                    // строка для печати на экран
	map<string, MyJsonObject> slovar;    // словарь, куда складываем фрагменты строки
	int begin_index;                     // номер записи в slovar, начиная с которой нужно собирать результирующий массив (самая большая, самая внешняя строка)


	if (in_string.length() == 0) {
		cout << " !!! На вход передана пустая строка. Её нельзя распарсить.";
		return;
	}

	// типы скобок
	string  skobka1_beg = "{",
		skobka1_end = "}",
		skobka2_beg = "[",
		skobka2_end = "]";

	pravilnaya_skobka(in_string);

	// обрабатываем фигурные скобки
	obrabotat_skobki(slovar, in_string, skobka1_beg, skobka1_end);
	begin_index = slovar.size();

	cout << endl << "Нужно собирать результирующий массив с ключа: @#" << begin_index << "#@" << endl;
	if (in_string != "@#" + to_string(begin_index) + "#@") {
		cout << in_string << endl;
		cout << "@#" + to_string(begin_index) + "#@" << endl;
		cout << endl << " !!! На вход передана неправильная строка. Дальнейшая обработка невозможна." << endl;
		return;
	}

	// в этой точке у нас есть строка и заполненный ассоциативный массив
	// единственное но - в значениях ассоциативного массива могут встречаться скобки другого вида
	// нужно это исправить
	// нужно пройтись по значениям массива и заменить скобки на указатели, при этом добавить
	// новые указатели в конец ассоциативного массива

	// печатаем словарь
	print_slovar(slovar);

	cout << endl << endl;

	// обходим заполненный ассоциативный массив, теперь обрабатываем квадратные скобки
	for (map<string, MyJsonObject>::iterator it = slovar.begin(); it != slovar.end(); ++it) {
		string value_string = it->second.unprocessed;  // делаем копию значения
		cout << "Обрабатываем строку: " << value_string << endl;
		string value_string_prev = value_string;
		obrabotat_skobki(slovar, value_string, skobka2_beg, skobka2_end);  // здесь выбираем другие скобки
		if (value_string != value_string_prev) {  // были изменения после нашего вызова
			cout << "  Есть изменения" << endl << endl;
			MyJsonObject obj; // задаём тип поля и значение поля
			obj.type_of_value = 0;
			obj.unprocessed = value_string;
			slovar[it->first] = obj;  // обновили значение в ассоциативном массиве

		}
		else {
			cout << "  Нет изменений" << endl << endl;
		}
	}

	// печатаем словарь
	print_slovar(slovar);

	// в этой точке у нас есть полностью разобранная json-строка


	// теперь начинаем "разматывать" наш ассоциативный массив в обратном направлении
	// точка входа - "@#" + to_string(begin_index) + "#@"  (или же то, что осталось лежать в in_string после всех замен)
	// когда внутри находим указатель, ищем его в массиве, подставляем значение
	// идём в цикле, пока в строке не останется ни одного указателя
	// параллельно со строкой собираем map/list


	// обходим нашу словарь в цикле
	// рядом строим точно такой же словарь, но вместо строк значениями будут map или list
	for (map<string, MyJsonObject>::iterator it = slovar.begin(); it != slovar.end(); ++it) {
		string val = trim_copy(it->second.unprocessed);          
		string substr = val.substr(1, val.length() - 2);         // убрали скобки
		vector<string> substr_pairs = split(trim_copy(substr), ',');  

		// словарь или список?
		if (val.substr(0, 1) == "{") {         
			map<string, MyJsonObject> my_map;  


			// обходим пары в формате "ключ: значение" цикле
			for (vector<string>::iterator value_pair = substr_pairs.begin(); value_pair != substr_pairs.end(); ++value_pair) {
				string tmp_str = value_pair->c_str();             
				vector<string> pair_elems = split(tmp_str, ':');  
				string posle_dvoetochiya = trim_copy(pair_elems[1]);   


				MyJsonObject out_elem = translate_string_to_obj(posle_dvoetochiya); 

				string my_key = remove_quotes(pair_elems[0]);
				my_map.insert(pair<string, MyJsonObject>(my_key, out_elem));
			}

			// записываем полученный map обратно в slovar
			it->second.my_map = my_map;
			it->second.unprocessed = "";   
			it->second.type_of_value = 8;  

		}
		else if (val.substr(0, 1) == "[") {  
			list<MyJsonObject> my_list;        

			// обходим элементы списка в цикле
			for (vector<string>::iterator value = substr_pairs.begin(); value != substr_pairs.end(); ++value) {
				string tmp_str = value->c_str();                           
				MyJsonObject out_elem = translate_string_to_obj(tmp_str);  
				my_list.push_back(out_elem);                               
			}

			// записываем полученный list обратно в slovar
			it->second.my_list = my_list;
			it->second.unprocessed = "";   
			it->second.type_of_value = 7;  
		}
	}

	
	string beg_key = "@#" + to_string(begin_index) + "#@";

	process_map(slovar, beg_key);  

	list<MyJsonObject> empty_list;  

	print_slovar2_map(0, slovar[beg_key].my_map, empty_list, 0);  

	result_obj = slovar[beg_key];  
}



int main() {
	setlocale(0, "russian"); 
	string in_string;

	in_string = R"({"name": "Ирина", "family_name": "Сергеева", "age": 20, "address": {"city": "Москва", "street": "Первомайская", "house": "1", "flat": "123"}, "children": [{"name": "Александра", "age": 2}, {"name": "Павел", "age": 1}], "maternity_leave": true})";
	
	MyJsonObject result_obj;
	parse_json(in_string, result_obj);

	return 0; 
}
