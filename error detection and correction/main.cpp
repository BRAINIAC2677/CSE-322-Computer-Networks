#include <iostream>
#include <string>
#include <vector>
#include <bitset>
#include <math.h>
#include <assert.h>
#include <random>
using namespace std;

void pad(string &_data, int _m)
{
    int n = _m - _data.size() % _m;
    if (_data.size() % _m != 0)
    {
        for (int i = 0; i < n; i++)
        {
            _data += '~';
        }
    }
}

string get_ascii(string _data)
{
    string ascii;
    for (int i = 0; i < _data.size(); i++)
    {
        ascii += bitset<8>(_data[i]).to_string();
    }
    return ascii;
}

vector<string> encode(string _data, int _m)
{
    vector<string> datablock(_data.size() / _m);
    for (int i = 0; i < _data.size() / _m; i++)
    {
        datablock[i] = get_ascii(_data.substr(i * _m, _m));
    }
    return datablock;
}

string decode(vector<string> _datablock, int _m)
{
    string ret;
    for (int i = 0; i < _datablock.size(); i++)
    {
        assert(_datablock[i].size() % (_m * 8) == 0);
        for (int j = 0; j < _datablock[i].size(); j += 8)
        {
            ret += char(bitset<8>(_datablock[i].substr(j, 8)).to_ulong());
        }
    }
    return ret;
}

int get_number_of_checkbits(int _number_of_databits)
{
    int r = 1;
    while ((1 << (r - 1)) < _number_of_databits + r)
    {
        r++;
    }
    return r - 1;
}

void init_checkbits(string &_datarow)
{
    int r = get_number_of_checkbits(_datarow.size());
    for (int j = 0; j < r; j++)
    {
        _datarow.insert(pow(2, j) - 1, "2");
    }
}

void calc_checkbits(string &_datarow)
{
    int r = get_number_of_checkbits(_datarow.size());
    for (int i = 0; i < r; i++)
    {
        int count = 0;
        for (int j = 0; j < _datarow.size(); j++)
        {
            if (_datarow[j] == '1' && (j + 1) & (1 << i))
            {
                count++;
            }
        }
        _datarow[pow(2, i) - 1] = count % 2 + '0';
    }
}

void add_checkbits(vector<string> &_datablock)
{
    for (int i = 0; i < _datablock.size(); i++)
    {
        init_checkbits(_datablock[i]);
        calc_checkbits(_datablock[i]);
    }
}

string serialize_datablock(vector<string> _datablock)
{
    string ret;
    int n = _datablock.size();
    if (n)
    {
        int m = _datablock[0].size();
        for (int i = 0; i < m; i++)
        {
            for (int j = 0; j < n; j++)
            {
                ret += _datablock[j][i];
            }
        }
    }
    return ret;
}

vector<string> deserialize_datablock(string _data, int _m)
{
    assert(_data.size() % _m == 0);
    int n = _data.size() / _m;
    vector<string> ret(n, string(_m, ' '));
    for (int i = 0; i < _m; i++)
    {
        for (int j = 0; j < n; j++)
        {
            ret[j][i] = _data[n * i + j];
        }
    }
    return ret;
}

string get_xor(string a, string b)
{
    assert(a.size() == b.size());
    string ret;
    for (int i = 0; i < a.size(); i++)
    {
        ret += (a[i] == b[i] ? '0' : '1');
    }
    return ret;
}

string append(string a, int n)
{
    for (int i = 0; i < n; i++)
    {
        a += '0';
    }
    return a;
}

string get_remainder(string _data, string _generator)
{
    int r = _generator.size() - 1;
    for (int i = 0; i < _data.size() - r; i++)
    {
        if (_data[i] == '1')
        {
            _data.replace(i, r + 1, get_xor(_data.substr(i, r + 1), _generator));
        }
    }
    string remainder = _data.substr(_data.size() - r);
    return remainder;
}

string get_crc_remainder(string _data, string _generator)
{
    return get_remainder(append(_data, _generator.size() - 1), _generator);
}

void add_crc_checksum(string &_data, string _generator)
{
    string crc_remainder = get_crc_remainder(_data, _generator);
    _data += crc_remainder;
}

void remove_crc_checksum(string &_data, string _generator)
{
    int r = _generator.size() - 1;
    if (r)
    {
        _data.resize(_data.size() - r);
    }
}

string simulate_transmission(string _data, double _p)
{
    string ret;
    for (int i = 0; i < _data.size(); i++)
    {
        double rand = (double)std::rand() / RAND_MAX;
        ret += (rand < _p ? (_data[i] == '1' ? '0' : '1') : _data[i]);
    }
    return ret;
}

bool has_error(string _data, string _generator)
{
    string remainder = get_remainder(_data, _generator);
    for (int i = 0; i < remainder.size(); i++)
    {
        if (remainder[i] == '1')
        {
            return true;
        }
    }
    return false;
}

int calc_error_syndrome(string _data)
{
    int ret = 0, i = 0;
    while ((1 << i) < _data.size())
    {
        int count = 0;
        for (int j = 0; j < _data.size(); j++)
        {
            if (_data[j] == '1' && (j + 1) & (1 << i))
            {
                count++;
            }
        }
        ret += ((count % 2) << i);
        i++;
    }
    return ret;
}

void toggle_error_bit(string &_data)
{
    int error_syndrome = calc_error_syndrome(_data);
    if (error_syndrome && error_syndrome < _data.size())
    {
        _data[error_syndrome - 1] = (_data[error_syndrome - 1] == '1' ? '0' : '1');
    }
}

void remove_checkbits(string &_data)
{
    int i = 0;
    while ((1 << i) < _data.size())
    {
        i++;
    }
    i--;
    while (i >= 0)
    {
        _data.erase(_data.begin() + (1 << i) - 1);
        i--;
    }
}

void correct_error(vector<string> &_datablock)
{
    for (int i = 0; i < _datablock.size(); i++)
    {
        toggle_error_bit(_datablock[i]);
        remove_checkbits(_datablock[i]);
    }
}

int main()
{
    string sent_data;
    int m;
    double p;
    string generator;

    cout << "enter data string: ";
    getline(cin, sent_data);

    cout << "enter number of data bytes in a row <m>: ";
    cin >> m;

    cout << "enter probability <p>: ";
    cin >> p;

    cout << "enter generator polynomial: ";
    cin >> generator;

    pad(sent_data, m);

    cout << "\ndata string after padding: " << sent_data << "\n";

    vector<string> sent_datablock = encode(sent_data, m);

    cout << "\ndata block <ascii code of m characters per row>:\n";
    for (int i = 0; i < sent_datablock.size(); i++)
    {
        for (int j = 0; j < sent_datablock[i].size(); j++)
        {
            cout << sent_datablock[i][j];
        }
        cout << "\n";
    }

    add_checkbits(sent_datablock);

    cout << "\ndata block after adding check bits:\n";
    for (int i = 0; i < sent_datablock.size(); i++)
    {
        for (int j = 0; j < sent_datablock[i].size(); j++)
        {
            if ((j & (j + 1)) == 0)
            {
                cout << "\033[1;32m" << sent_datablock[i][j] << "\033[1;0m";
            }
            else
            {
                cout << sent_datablock[i][j];
            }
        }
        cout << "\n";
    }

    string sent_frame = serialize_datablock(sent_datablock);
    cout << "\ndata bits after column-wise serialization:\n"
         << sent_frame << "\n";

    add_crc_checksum(sent_frame, generator);

    cout << "\ndata bits after adding CRC checksum <sent frame>:\n";
    for (int i = 0; i < sent_frame.size(); i++)
    {
        if (i >= sent_frame.size() - generator.size() + 1)
        {
            cout << "\033[1;36m" << sent_frame[i] << "\033[1;0m";
        }
        else
        {
            cout << sent_frame[i];
        }
    }

    string received_frame = simulate_transmission(sent_frame, p);

    cout << "\n\nreceived frame:\n";
    for (int i = 0; i < received_frame.size(); i++)
    {
        if (received_frame[i] != sent_frame[i])
        {
            cout << "\033[1;31m" << received_frame[i] << "\033[1;0m";
        }
        else
        {
            cout << received_frame[i];
        }
    }
    cout << "\n";

    cout << "\nresult of CRC checksum matching: " << (has_error(received_frame, generator) ? "error detected" : "no error detected") << "\n";

    remove_crc_checksum(received_frame, generator);

    vector<string> received_datablock = deserialize_datablock(received_frame, m * 8 + get_number_of_checkbits(m * 8));

    cout << "\ndata block after removing CRC checksum bits:\n";
    for (int i = 0; i < received_datablock.size(); i++)
    {
        for (int j = 0; j < received_datablock[i].size(); j++)
        {

            if (received_datablock[i][j] != sent_frame[j * received_datablock.size() + i])
            {
                cout << "\033[1;31m" << received_datablock[i][j] << "\033[1;0m";
            }
            else
            {
                cout << received_datablock[i][j];
            }
        }
        cout << "\n";
    }

    correct_error(received_datablock);

    cout << "\ndata block after removing check bits:\n";
    for (int i = 0; i < received_datablock.size(); i++)
    {
        for (int j = 0; j < received_datablock[i].size(); j++)
        {
            cout << received_datablock[i][j];
        }
        cout << "\n";
    }

    string received_data = decode(received_datablock, m);

    cout << "\noutput frame: " << received_data << "\n";

    return 0;
}