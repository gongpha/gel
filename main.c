#include "gelc_tokenizer.h"
int main() {
	const wchar_t* source = L"hello 123 \"setyor\" omega 0.1";
	gelc_tokenizer t;
	gelc_tokenizer_create(&t, source);
	while (gelc_tokenizer_read(&t)) {
	}
	return 0;
}