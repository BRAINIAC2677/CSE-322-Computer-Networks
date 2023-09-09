#include <bits/stdc++.h>
using namespace std;

void pad(string &_data, int _m)
{
    if (_data.size() % _m != 0)
    {
        for (int i = 0; i < _m - _data.size() % _m; i++)
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

int get_number_of_checkbits(int _number_of_databits)
{
    int r = 1;
    while (pow(2, r - 1) < _number_of_databits + r)
    {
        r++;
    }
    return r - 1;
}

void print_datablock(vector<string> _datablock)
{
    for (int i = 0; i < _datablock.size(); i++)
    {
        cout << _datablock[i] << "\n";
    }
}

void init_checkbits(string &_datablock)
{
    int r = get_number_of_checkbits(_datablock.size());
    for (int j = 0; j < r; j++)
    {
        _datablock.insert(pow(2, j) - 1, "2");
    }
}

void calc_checkbits(string &_datablock)
{
    int r = get_number_of_checkbits(_datablock.size());
    for (int i = 0; i < r; i++)
    {
        int count = 0;
        for (int j = 0; j < _datablock.size(); j++)
        {
            if (_datablock[j] == '1' && (j + 1) & (1 << i))
            {
                count++;
            }
        }
        _datablock[pow(2, i) - 1] = count % 2 + '0';
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
        cout << "_datablock[0].size(): " << _datablock[0].size() << "\n";
        for (int i = 0; i < m; i++)
        {
            for (int j = 0; j < n; j++)
            {
                ret += _datablock[j][i];
            }
            cout << "i: " << i << " ret.size(): " << ret.size() << "\n";
        }
    }
    cout << "ret.size(): " << ret.size() << "\n";
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
    cout << "generator: " << _generator << "\n";
    int r = _generator.size() - 1;
    for (int i = 0; i < _data.size() - r; i++)
    {
        if (_data[i] == '1')
        {
            _data.replace(i, r + 1, get_xor(_data.substr(i, r + 1), _generator));
        }
    }
    return _data.substr(_data.size() - r);
}

string get_crc_remainder(string _data, string _generator)
{
    cout << "generator: " << _generator << "\n";
    return get_remainder(append(_data, _generator.size() - 1), _generator);
}

void add_crc_checksum(string &_data, string _generator)
{
    cout << "generator: " << _generator << "\n";
    string crc_remainder = get_crc_remainder(_data, _generator);
    cout << "\ncrc remainder: " << crc_remainder << "\n";
    _data += crc_remainder;
}

bool same(string _a, string _b)
{
    if (_a.size() != _b.size())
    {
        cout << "_a.size(); " << _a.size() << " _b.size(): " << _b.size() << "\n";
        return false;
    }
    for (int i = 0; i < _a.size(); i++)
    {
        if (_a[i] != _b[i])
        {
            cout << "i: " << i << " _a[i]: " << _a[i] << " _b[i]: " << _b[i] << "\n";
            return false;
        }
    }
    return true;
}

int main()
{
    string d = "010110001011000000000001111111011101000000110001111000010101000110011110000000111011111111111000000000011010011011001010110011";
    string g = "10101";

    string data_string = "Hamming Code";
    int m = 2;
    string generator = "10101";

    pad(data_string, m);
    vector<string> datablock = encode(data_string, m);
    add_checkbits(datablock);
    string serialized_datablock = serialize_datablock(datablock);

    cout << d << endl;
    cout << serialized_datablock << endl;

    cout << "d and serialized_datablock are equal: " << (same(d, serialized_datablock) == false) << "\n\n";

    add_crc_checksum(d, g);
    add_crc_checksum(serialized_datablock, generator);

    cout << d << endl;
    cout << serialized_datablock << endl;

    return 0;
}