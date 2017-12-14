#ifndef     _CSVPARSER_HPP_
#define     _CSVPARSER_HPP_

#include <stdexcept>
#include <string>
#include <vector>
#include <list>
#include <sstream>

/*
i: int32
u: uint32
c: uint8
h: uint16
l: uint64
d: double
f: float
b: bool
s: string
*/

namespace csv {

    class Error : public std::runtime_error {
    public:
        Error(const std::string &msg) :
            std::runtime_error(std::string("CSVparser : ").append(msg))
        {
        }
    };



    class Row {
    private:
        const std::vector<std::string>& _header;
        std::vector<std::string>        _values;

    public:
        Row(const std::vector<std::string> &);
        ~Row(void);

        size_t size(void) const;
        void push(const std::string &);
        bool set(const std::string &, const std::string &);

        const std::string& operator[](unsigned int) const;
        const std::string& operator[](const std::string &valueName) const;
        friend std::ostream& operator<<(std::ostream& os, const Row &row);
        friend std::ofstream& operator<<(std::ofstream& os, const Row &row);

        template<class T> const T getValue(unsigned int pos) const
        {
            if (pos < _values.size()) {
                T res;
                std::stringstream ss;
                ss << _values[pos];
                ss >> res;
                return res;
            }
            throw Error("can't return this value (doesn't exist)");
        }
        template<class T> void Parse(T* p) const
        {
            int offset = 0; char* head = (char*)p;
            const char* pFormat = T::GetFormat();
            for (size_t i = 0; i < strlen(pFormat); ++i) {
                switch (pFormat[i]) {
                case 'i':
                    *(int32*)(head + offset) = atoi(_values[i].c_str());
                    offset += sizeof(int32);
                    break;
                case 'u':
                    *(uint32*)(head + offset) = atoi(_values[i].c_str());
                    offset += sizeof(uint32);
                    break;
                case 'c':
                    *(uint8*)(head + offset) = atoi(_values[i].c_str());
                    offset += sizeof(uint8);
                    break;
                case 'h':
                    *(uint16*)(head + offset) = atoi(_values[i].c_str());
                    offset += sizeof(uint16);
                    break;
                case 'l':
                    *(uint64*)(head + offset) = atoll(_values[i].c_str());
                    offset += sizeof(uint64);
                    break;
                case 'd':
                    *(double*)(head + offset) = atof(_values[i].c_str());
                    offset += sizeof(double);
                    break;
                case 'f':
                    *(float*)(head + offset) = (float)atof(_values[i].c_str());
                    offset += sizeof(float);
                    break;
                case 'b':
                    *(bool*)(head + offset) = atoi(_values[i].c_str());
                    offset += sizeof(bool);
                    break;
                case 's':
                    *(std::string*)(head + offset) = _values[i];
                    offset += sizeof(std::string);
                    break;
                default:
                    break;
                }
            }
            assert(sizeof(T) >= offset);
        }
    };

    enum DataType {
        eFILE = 0,
        ePURE = 1
    };

    class Parser {
    private:
        std::string     _file;
        const DataType  _type;
        const char      _sep;
        std::vector<std::string> _originalFile;
        std::vector<std::string> _header;
        std::vector<Row*> _content;

    public:
        Parser(const std::string &, const DataType &type = eFILE, char sep = ',');
        ~Parser(void);

        Row &getRow(unsigned int row) const;
        Row &operator[](unsigned int row) const;
        size_t rowCount(void) const;
        size_t columnCount(void) const;
        std::vector<std::string> getHeader(void) const;
        const std::string getHeaderElement(unsigned int pos) const;
        const std::string &getFileName(void) const;

    public:
        bool deleteRow(unsigned int row);
        bool addRow(unsigned int pos, const std::vector<std::string> &);
        void sync(void) const;

    protected:
        void parseHeader(void);
        void parseContent(void);
    };
    
    template<class T> void LoadCsv(std::vector<T>& table, const char* name)
    {
        std::string csvname("../data/csv/");
        csvname.append(name);
        T data;
        csv::Parser file(csvname);
        uint cnt = file.rowCount();
        for (uint i = 0; i < cnt; ++i) {
            if (file[i][0].at(0) == '#') continue;
            file[i].Parse(&data);
            table.push_back(data);
        }
    }
    template<class Key, class Val> void LoadCsv(std::map<Key, Val>& table, const char* name)
    {
        std::string csvname("../data/csv/");
        csvname.append(name);
        Val data;
        Key* pKey = (Key*)(char*)&data;
        csv::Parser file(csvname);
        uint cnt = file.rowCount();
        for (uint i = 0; i < cnt; ++i) {
            if (file[i][0].at(0) == '#') continue;
            file[i].Parse(&data);
            assert(table.find(*pKey) == table.end());
            table.insert(std::make_pair(*pKey, data));
        }
    }
}

#endif /*!_CSVPARSER_HPP_*/
