#include <iostream>
#include <cstdint>
#include <cstring>
#include <assert.h>
#include <fstream>
#include <climits>
#include <sys/stat.h>

#include "TABLES.h"

int is_int_overflow (int64_t si_a, int64_t si_b);

union value {
    int64_t int_n;
    double float_n;
    uint8_t bin[NUM_SIZE];
};

#include "Stack.h"

const char ERROR[] = "What!? ";

class Processor {
public:
    value RX0 = {};
    value RX1 = {};
    value RX2 = {};
    value RX3 = {};
    value RX4 = {};
    value RX5 = {};
    value RX6 = {};
    value RX7 = {};
    value RX8 = {};
    value RX9 = {};
    value RXA = {};
    value RXB = {};
    value RXC = {};
    value RXD = {};
    value RXE = {};
    value RXF = {};
    const uint8_t *RAM;

    Stack St;
    int argc;
    char **argv;
    FILE *prg;
    uint8_t *assm;
    size_t assm_ptr;

    size_t file_size;

    int init (int argc_in, char **argv_in);
    int print_help ();
    int file_init ();
    int destroy ();
    size_t get_file_size_LINUX(char* filename);
    value *find_reg (C_REG_T regisrty);

    int exec ();
};

int main (int argc, char **argv)
{
    Processor processor;
    if (processor.init (argc, argv))
        return 1;
    processor.exec ();
    processor.destroy ();
    return 0;
}

int Processor::init (int argc_in, char **argv_in)
{
    this->argc = argc_in;
    this->argv = argv_in;
    int is_opened = this->file_init ();
    if (is_opened) {
        return is_opened;
    }
    this->RAM = NULL;
    this->RAM = new uint8_t[RAM_SIZE];
    this->St.init ();
    this->assm_ptr = 0;
    return 0;
}

int Processor::destroy ()
{
    this->St.destroy ();
    delete[] this->assm;
    delete[] this->RAM;
    return 0;
}

int Processor::file_init ()
{
    if (this->argc == 1 || strcmp (this->argv[1], "-h") == 0 || strcmp (this->argv[1], "--help") == 0) {
        this->print_help();
        return 1;
    }
    this->prg = fopen (this->argv[1], "rb");
    if (prg == NULL) {
        std::cout << "Can not open input file: \"" << argv[1] << "\"" << std::endl;
        return 1;
    }
    this->file_size = this->get_file_size_LINUX (argv[1]);
    this->assm = new uint8_t[this->file_size];
    fread (this->assm, sizeof (uint8_t), this->file_size, this->prg);
    fclose (this->prg);
    return 0;
}

int Processor::print_help ()
{
    std::cout << "Usage:" << std::endl;
    std::cout << "./procc [binary code file]" << std::endl;
    return 0;
}

size_t Processor::get_file_size_LINUX(char* filename)
{
      /**
        Gets file size in linux-based OS
        @param[in] filename (char*) no comments)
        @return File size
        */

        assert (filename != NULL);

    struct stat st;
    stat (filename, &st);
    return st.st_size;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////

int Processor::exec ()
{
    for ( ; this->assm[this->assm_ptr] != 0x50; this->assm_ptr += 1) {
        value val = {};
        size_t tmp;
        value* p_val = NULL;
        value a = {}, b = {};
        switch (this->assm[this->assm_ptr]) {
        case PUSH:
            this->assm_ptr += 1;
            switch (this->assm[this->assm_ptr]) {
            case REG:
                this->assm_ptr += 1;
                p_val = this->find_reg(this->assm[this->assm_ptr]);
                if (p_val != NULL)
                    this->St.push (*p_val);
                else {
                    std::cout << ERROR << "@push-bad-reg_num" << std::endl;
                    return 1;
                }
                break;
            case INUM:
                memcpy (val.bin, &this->assm[this->assm_ptr + 1], NUM_SIZE * sizeof (uint8_t));
                //std::cout << "push_inum " << val.int_n << std::endl;
                this->assm_ptr += NUM_SIZE;
                this->St.push (val);
                break;
            default:
                std::cout << ERROR << "@push-bad-flag" << std::endl;
                return 1;
            }
            break;
        case POP:
            this->assm_ptr += 1;
            p_val = this->find_reg(this->assm[this->assm_ptr]);
            if (p_val != NULL) {
                if (this->St.pop (p_val) == STACK_UNDERFLOW_ERROR) {
                    std::cout << ERROR << "@pop-underflow" << std::endl;
                    return 1;
                }
            }
            else {
                std::cout << ERROR << "@pop-bad-reg_num" << std::endl;
                return 1;
            }
            break;
        case IN:
            this->assm_ptr += 1;
            p_val = this->find_reg(this->assm[this->assm_ptr]);
            if (p_val != NULL)
                std::cin >> (*p_val).int_n;
            else {
                std::cout << ERROR << "@in-bad-reg_num" << std::endl;
                return 1;
            }
            break;
        case OUT:
            this->assm_ptr += 1;
            p_val = this->find_reg(this->assm[this->assm_ptr]);
            if (p_val != NULL)
                std::cout << (*p_val).int_n << std::endl;
            else {
                std::cout << ERROR << "@in-bad-reg_num" << std::endl;
                return 1;
            }
            break;
        case SUM:
            this->St.pop (&a);
            this->St.pop (&b);
            if (this->St.Status != STACK_UNDERFLOW_ERROR) {
                a.int_n += b.int_n;
                this->St.push (a);
            }
            else {
                std::cout << ERROR << "@sum-underflow" << std::endl;
                return 1;
            }
            break;
        case SUB:
            this->St.pop (&a);
            this->St.pop (&b);
            if (this->St.Status != STACK_UNDERFLOW_ERROR) {
                b.int_n -= a.int_n;
                this->St.push (b);
            }
            else {
                std::cout << ERROR << "@sub-underflow" << std::endl;
                return 1;
            }
            break;
        case MUL:
            this->St.pop (&a);
            this->St.pop (&b);
            if (this->St.Status != STACK_UNDERFLOW_ERROR) {
                a.int_n *= b.int_n;
                this->St.push (a);
            }
            else {
                std::cout << ERROR << "@mul-underflow" << std::endl;
                return 1;
            }
            break;
        case DIV:
            this->St.pop (&a);
            this->St.pop (&b);
            if (this->St.Status != STACK_UNDERFLOW_ERROR) {
                b.int_n /= a.int_n;
                this->St.push (b);
            }
            else {
                std::cout << ERROR << "@div-underflow" << std::endl;
                return 1;
            }
            break;
        case S_S:
            return 0;
        case L0:
            if (this->RX0.int_n <= 1) {
                this->assm_ptr += NUM_SIZE;
                break;
            }
            memcpy (&tmp, &this->assm[this->assm_ptr + 1], NUM_SIZE * sizeof (uint8_t));
            this->assm_ptr = tmp - 1;
            this->RX0.int_n -= 1;
            break;
        case L1:
            if (this->RX1.int_n <= 1) {
                this->assm_ptr += NUM_SIZE;
                break;
            }
            memcpy (&tmp, &this->assm[this->assm_ptr + 1], NUM_SIZE * sizeof (uint8_t));
            this->assm_ptr = tmp - 1;
            this->RX1.int_n -= 1;
            break;
        case JMP:
            memcpy (&tmp, &this->assm[this->assm_ptr + 1], NUM_SIZE * sizeof (uint8_t));
            this->assm_ptr = tmp - 1;
            break;
        default:
            std::cout << ERROR << "@exec-bad-instruction" << std::endl;
            return 1;
            break;
        }
    }
    return 0;
}

value * Processor::find_reg (C_REG_T regisrty)
{
    switch (regisrty) {
    case X0:
        return &this->RX0;
        break;
    case X1:
        return &this->RX1;
        break;
    case X2:
        return &this->RX2;
        break;
    case X3:
        return &this->RX3;
        break;
    case X4:
        return &this->RX4;
        break;
    case X5:
        return &this->RX5;
        break;
    case X6:
        return &this->RX6;
        break;
    case X7:
        return &this->RX7;
        break;
    case X8:
        return &this->RX8;
        break;
    case X9:
        return &this->RX9;
        break;
    case XA:
        return &this->RXA;
        break;
    case XB:
        return &this->RXB;
        break;
    case XC:
        return &this->RXC;
        break;
    case XD:
        return &this->RXD;
        break;
    case XE:
        return &this->RXE;
        break;
    case XF:
        return &this->RXF;
        break;
    default:
        return NULL;
        break;
    }
}
