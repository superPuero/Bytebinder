#include <iostream>
#include <fstream>
#include <string>
#include <type_traits>
	
#define TO_STRING(x) #x
	
class Bytebinder {

public:

	//file - file name to save data to
	Bytebinder(const char* file) {
		m_savefile = file;
	}

	// Saves a raw binary data of an object under specific key 
	// Key is saved in format: true_key = type + key;
	template<typename Ty>
	void put(Ty* object, const char* key) {
		std::ofstream outfile(m_savefile, std::ios::binary | std::ios::app);

		std::string true_key = typeid(Ty).name();
		true_key += key;
		true_key += '\n';

		outfile.write(true_key.c_str(), true_key.size());
		outfile.write(reinterpret_cast<char*>(object), sizeof(Ty));
		outfile.put('\n');

		outfile.close();
	}

	// Returns a pointer of right type to dynamically allocatad copy of data assosiated with key
	// Use delete after using
	template<typename Ty>
	[[nodiscard]] Ty* get_new(const char* key) {
		std::ifstream infile(m_savefile, std::ios::binary);

		std::string buff;

		std::string true_key = typeid(Ty).name();
		true_key += key;

		while (getline(infile, buff)) {
			if (buff == true_key) {

				Ty* obj = nullptr;
				while (!obj) {
					obj = static_cast<Ty*>(malloc(sizeof(Ty)));
				}

				infile.read(reinterpret_cast<char*>(obj), sizeof(Ty));
				infile.close();

				return obj;
			}
		}
		infile.close();

		std::cout << "didnt found key " << true_key << "\n";

		return nullptr;
	}

	//Returns a temporary stack allocatad copy of data assosiated with key
	template<typename Ty>
	[[nodiscard]] Ty get_temp(const char* key){
		std::ifstream infile(m_savefile, std::ios::binary);

		std::string buff;
		std::string true_key = typeid(Ty).name();
		true_key += key;

		while (getline(infile, buff)) {
			if (buff == true_key) {
				char obj[sizeof(Ty)];

				infile.read(obj, sizeof(Ty));
				infile.close();

				return *reinterpret_cast<Ty*>(&obj);
			}
		}

		infile.close();

		std::cout << "key: " << true_key << " wasnt found" << '\n';

		__debugbreak();

	}
	//Clears a savefile
	void clear() {		
		std::ofstream outfile(m_savefile, std::ios::binary);

		outfile.clear();

		std::cout << "file cleared" << '\n';
	}
private:
	const char* m_savefile;

};
	
struct Foo {
	Foo(int num1, float num2) {
		number1 = num1;
		number2 = num2;
	}

	int number1;
	float number2;

};
	
int main(){
	Foo* foo1 = new Foo(12, 56.74f);
	Foo foo2(64, 34.5f);

	Bytebinder binder("test.txt");

	binder.put(foo1, "foo1");
	binder.put(&foo2, "foo2");

	Foo	new_foo1 = binder.get_temp<Foo>("foo1");
	Foo* new_foo2 = binder.get_new<Foo>("foo2");

	std::cout << "new_foo1:" << '\n';
	std::cout << new_foo1.number1 << '\n';
	std::cout << new_foo1.number2 << '\n';
	std::cout << '\n';


	std::cout << "new_foo2:" << '\n';
	std::cout << "ptr: " << new_foo2 << '\n';
	std::cout << new_foo2->number1 << '\n';
	std::cout << new_foo2->number2 << '\n';
	std::cout << '\n';

	binder.clear();


}	