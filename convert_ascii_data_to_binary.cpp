#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <string>

using namespace std;

/*
 * Issue with ofstream open not accepting a C++ string.
 * https://stackoverflow.com/questions/22567256/how-to-fix-no-matching-function-ifstream-error
 */

class ConvertData
{
    public:
        void convert(string infile, string outfile) {
            int count = 0;
            ifstream fin;
            fin.open(infile.c_str(), ios::in);

            ofstream fout;
            fout.open(outfile.c_str(), ios::binary);

            string line;
            int16_t num;
            while (fin >> line) {
                num = atoi(line.c_str());
                fout.write((char*)&num, sizeof(num));
                printf("%x\n", num);
                count++;
            }
            cout << "size of conversion:" << sizeof(num) << endl;
            cout << count << " numbers converted." << endl;
        }

};

int main()
{
    string input_data_path = "data.csv";
    string output_data_path = "data.bin";

    ConvertData convert_data;
    convert_data.convert(input_data_path, output_data_path);
}
