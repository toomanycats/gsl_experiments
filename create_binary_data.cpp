#include <iostream>
#include <fstream>
#include <vector>
#include <string>

using namespace std;

class ConvertData
{
    public:
        void load_data_from_file(string infile, vector<string> &rows)
		{
			int count;
			ifstream fin;
			string line;

			fin.open(infile.c_str(), ios::in);
			while (fin >> line) {
				//rows.clear();
				getline(fin, line);
				rows.push_back(line);
				count++;
			}
			cout << count << " lines counted." << endl;
		}

        void write_rows_to_binary(string outfile, vector<string> &rows){
            vector<string>::iterator it;

            ofstream fout;
            fout.open(outfile.c_str(), fstream::binary);

            for(it = rows.begin(); it != rows.end(); it++) {
                fout << (char*) &it;
            }

            fout.close();
        }
};

int main()
{
    string input_data_path = "data.csv";
    string output_data_path = "data.bin";
    vector<string> rows;

    ConvertData convert_data;

    convert_data.load_data_from_file(input_data_path, rows);
    convert_data.write_rows_to_binary(output_data_path, rows);
}
