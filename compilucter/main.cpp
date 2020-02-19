#include <iostream>
#include <cstdint>
#include <cstring>
#include <assert.h>

#include <sys/stat.h>

#include "TABLES.h"

const char* DEF_OUT_NAME = "a.prg";
const size_t DEF_LEXEM_MAS_SIZE = 100;
const size_t DEF_LABEL_MAS_SIZE = 100;
const size_t DEF_OUT_CAPACITY = 100;

union value {
    int64_t int_n;
    double float_n;
    uint8_t bin[NUM_SIZE];
};

union label_addr {
    size_t addr;
    uint8_t bin[sizeof (size_t)];
};

struct lexem {
    FLAG_T flag;
    CMD_T cmd;
    REG_T reg;
    value val;
    char *origin;
    char *label_name; //or function name
};

struct label {
    char* name;
    size_t place;
};

struct label_res {
    size_t lex_num;
    size_t out_num;
};

class Compilucter {
public:
    const char *delim = "\t \n";

    struct lexem *lexem_mas;
    size_t lexem_mas_size;
    size_t cur_lexem;

    struct label *label_mas;
    void check_label_mas ();
    size_t label_mas_size;
    size_t cur_label;

    struct label_res *labels_resolver;
    void check_labels_resolver ();
    size_t labels_resolver_size;
    size_t cur_labels_resolver;

    int argc;
    char **argv;
    FILE *src;
    size_t file_size;
    char *code;
    char *code_orig;
    size_t num_line;
    size_t num_word;

    FILE *out;
    uint8_t *out_mas;
    size_t out_size;
    size_t out_capacity;

    int out_write ();
    int init (int argc_in, char **argv_in);
    void check_lexem_mas ();
    void check_out_mas ();
    void out_push (uint8_t byte);
    size_t get_file_size_LINUX(char* filename);
    int find_label (char *name, size_t *place);
    int file_init ();
    int print_help ();
    int parse ();
    int destroy ();
    int is_num (char *str, int64_t *num);
    CMD_T is_cmd (char *str);
    CMD_T is_cmd_if (char* str);
    REG_T is_reg (char* str);
    int is_comment (char* str);
    char *find_line (char *target, size_t *num_line);
    int compile ();
    void error_handle (char *cur_str, char *cause, const char *description);
};

int main(int argc, char **argv)
{
    Compilucter Compiler;
    if (Compiler.init (argc, argv))
        return 1;
    if (Compiler.parse ())
        Compiler.compile ();
    Compiler.destroy ();
    return 0;
}

int Compilucter::init (int argc_in, char **argv_in)
{
    this->argc = argc_in;
    this->argv = argv_in;
    bool is_opened = this->file_init ();
    if (is_opened)
        return is_opened;

    this->lexem_mas = NULL;
    this->lexem_mas = new struct lexem[DEF_LEXEM_MAS_SIZE];
    for (size_t i = 0; i < DEF_LEXEM_MAS_SIZE; i++) {
        this->lexem_mas[i] = {};
    }
    this->lexem_mas_size = DEF_LEXEM_MAS_SIZE;
    this->cur_lexem = 0;;

    this->label_mas = NULL;
    this->label_mas = new struct label[DEF_LABEL_MAS_SIZE];
    for (size_t i = 0; i < DEF_LABEL_MAS_SIZE; i++) {
        this->label_mas[i] = {};
    }
    this->label_mas_size = DEF_LABEL_MAS_SIZE;
    this->cur_label = 0;

    this->out_mas = new uint8_t[DEF_OUT_CAPACITY];
    this->out_size = 0;
    this->out_capacity = DEF_OUT_CAPACITY;

    this->labels_resolver = NULL;
    this->labels_resolver = new struct label_res[DEF_LABEL_MAS_SIZE];
    this->labels_resolver_size = DEF_LABEL_MAS_SIZE;
    this->cur_labels_resolver = 0;
    for (size_t i = 0; i < DEF_LABEL_MAS_SIZE; i++) {
        this->labels_resolver[i] = {};
    }

    return 0;
}

int Compilucter::destroy ()
{
    delete[] this->code;
    delete[] this->code_orig;
    delete[] this->lexem_mas;
    delete[] this->label_mas;
    delete[] this->out_mas;
    delete[] this->labels_resolver;
    fclose (this->out);
    return 0;
}

int Compilucter::file_init ()
{
    char *OUT_NAME = (char *) DEF_OUT_NAME;
    if (this->argc == 1 || strcmp (this->argv[1], "-h") == 0 || strcmp (this->argv[1], "--help") == 0) {
        this->print_help();
        return 1;
    }
    if (this->argc > 2) {
        OUT_NAME = this->argv[2];
    }
    this->src = NULL;
    this->src = fopen (this->argv[1], "rt");
    if (this->src == NULL) {
        std::cout << "Can not open input file: " << this->argv[1] << std::endl;
        return 1;
    }
    this->out = NULL;
    this->out = fopen (OUT_NAME, "wb");
    if (this->out == NULL) {
        std::cout << "Can not open output file. Check permissions: " << OUT_NAME << std::endl;
        fclose (this->src);
        return 1;
    }
    this->file_size = this->get_file_size_LINUX (this->argv[1]);
    this->code = new char[this->file_size + 1];
    this->code_orig = new char[this->file_size + 1];
    fread (this->code, sizeof (char), this->file_size, this->src);
    memcpy (code_orig, code, this->file_size);
    this->code[this->file_size] = '\0';
    this->code_orig[this->file_size] = '\0';
    fclose (this->src);
    return 0;
}

int Compilucter::print_help ()
{
    std::cout << "Usage:" << std::endl;
    std::cout << "./compilucter [source code file] [output file]" << std::endl;
    return 0;
}

size_t Compilucter::get_file_size_LINUX(char* filename)
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

//////////////////////////////////RARSE_TO_LEXEMS////////////////////////////////////////////////////

int Compilucter::parse ()
{
    int lexically_correct = 1;
    char *tmp;
    if ((tmp = strtok (this->code, this->delim)) == NULL) {
        std::cout << "empty file" << std::endl;
    }
    while (tmp != NULL) {
        bool is_correct = 0;
        CMD_T cmd_tmp = 0x00;
        if ((cmd_tmp = this->is_cmd(tmp))) {
            this->lexem_mas[this->cur_lexem].cmd = cmd_tmp;
            this->lexem_mas[this->cur_lexem].flag = CMD;
            this->lexem_mas[this->cur_lexem].val.int_n = 0;
            this->lexem_mas[this->cur_lexem].origin = tmp;
            this->lexem_mas[this->cur_lexem].reg = 0x00;
            this->lexem_mas[this->cur_lexem].label_name = NULL;
            this->cur_lexem++;
            this->check_lexem_mas ();
            is_correct = 1;
        }
        if ((cmd_tmp = this->is_cmd_if(tmp))) {
            char *tmp_old = tmp;
            tmp = strtok (NULL, this->delim);
            if (tmp == NULL) {
                this->error_handle (tmp_old, tmp_old, "Conditional or jump operator needs a label name.");
                lexically_correct = 0;
                return 0;
            }
            else {
                this->lexem_mas[this->cur_lexem].cmd = cmd_tmp;
                this->lexem_mas[this->cur_lexem].flag = CMD;
                this->lexem_mas[this->cur_lexem].val.int_n = 0;
                this->lexem_mas[this->cur_lexem].origin = tmp_old;
                this->lexem_mas[this->cur_lexem].reg = 0x00;
                this->lexem_mas[this->cur_lexem].label_name = tmp;
                this->cur_lexem++;
                this->check_lexem_mas ();
                is_correct = 1;
            }
        }

        REG_T reg_tmp = 0x00;
        if ((reg_tmp = this->is_reg(tmp))) {
            this->lexem_mas[this->cur_lexem].reg = reg_tmp;
            this->lexem_mas[this->cur_lexem].flag = REG;
            this->lexem_mas[this->cur_lexem].val.int_n = 0;
            this->lexem_mas[this->cur_lexem].origin = tmp;
            this->lexem_mas[this->cur_lexem].cmd = 0x00;
            this->lexem_mas[this->cur_lexem].label_name = NULL;
            this->cur_lexem++;
            this->check_lexem_mas ();
            is_correct = 1;
        }

        int64_t int_tmp = 0;
        if (this->is_num(tmp, &int_tmp)) {
            this->lexem_mas[this->cur_lexem].cmd = 0x00;
            this->lexem_mas[this->cur_lexem].flag = INUM;
            this->lexem_mas[this->cur_lexem].val.int_n = int_tmp;
            this->lexem_mas[this->cur_lexem].origin = tmp;
            this->lexem_mas[this->cur_lexem].reg = 0x00;
            this->lexem_mas[this->cur_lexem].label_name = NULL;
            this->cur_lexem++;
            this->check_lexem_mas ();
            is_correct = 1;
        }

        if (!strcmp (tmp, "label")) {
            char *tmp_old = tmp;
            tmp = strtok (NULL, this->delim);
            if (tmp == NULL) {
                this->error_handle (tmp_old, tmp_old, "Label needs name!");
                lexically_correct = 0;
                return 0;
            }
            else {
                this->lexem_mas[this->cur_lexem].cmd = 0x00;
                this->lexem_mas[this->cur_lexem].flag = LABEL;
                this->lexem_mas[this->cur_lexem].val.int_n = 0;
                this->lexem_mas[this->cur_lexem].origin = tmp_old;
                this->lexem_mas[this->cur_lexem].reg = 0x00;
                this->lexem_mas[this->cur_lexem].label_name = tmp;
                this->cur_lexem++;
                this->check_lexem_mas ();
                is_correct = 1;
            }
        }

        if (is_comment (tmp))
            is_correct = 1;

        if (!is_correct) {
            this->error_handle (tmp, tmp, NULL);
            lexically_correct = 0;
        }

        tmp = strtok (NULL, this->delim);
    }
    return lexically_correct;
}

void Compilucter::error_handle (char *cur_str, char *cause, const char *description)
{
    size_t error_line = 0;
    char *error_str = this->find_line (cur_str, &error_line);
    size_t cause_line = 0;
    this->find_line (cause, &cause_line);
        std::cout << "Error in line " << error_line << ": \"";
        for (size_t i = 0; error_str[i] != '\n'; i++)
            printf ("%c", error_str[i]);
        std::cout << "\"" << std::endl;
    std::cout << "In phrase: \"" << cur_str;
    if (cause_line == error_line && cause != cur_str)
        std::cout << " " << cause << "\"" << std::endl;
    else
        std::cout << "\"" << std::endl;
    if (description != NULL)
        std::cout << description << std::endl;
    return;
}

void Compilucter::check_lexem_mas ()
{
    if (this->cur_lexem >= this->lexem_mas_size) {
        struct lexem *old = this->lexem_mas;
        this->lexem_mas_size *= 2;
        this->lexem_mas = new struct lexem[this->lexem_mas_size];
        memcpy (this->lexem_mas, old, this->cur_lexem * sizeof (struct lexem));
        for (size_t i = this->cur_lexem; i < this->lexem_mas_size; i++) {
            this->lexem_mas[i] = {};
        }
        delete[] old;
        return;
    }
    return;
}

int Compilucter::is_comment (char* str)
{
    // are you ready to see the worst code you`ve ever seen?
    size_t ptr = (size_t) str + strlen (str) - (size_t) this->code - 1; //offset, last symbol of char *str in ORIGINAL code
    if (str[0] == '/' && str[1] == '/') {
        if (*(this->code_orig + ptr + 1) != '\n')
            strtok (NULL, "\n");
        return 1;
    }
    if (str[0] == '/' && str[1] == '*') {
        if (*(this->code_orig + ptr) != '*')
            strtok (NULL, "*");
        return 1;
    }
    return 0;
}

CMD_T Compilucter::is_cmd (char* str)
{
    if (!strcmp (str, "push")) return PUSH;
    if (!strcmp (str, "pop")) return POP;
    if (!strcmp (str, "in")) return IN;
    if (!strcmp (str, "out")) return OUT;
    if (!strcmp (str, "sum")) return SUM;
    if (!strcmp (str, "sub")) return SUB;
    if (!strcmp (str, "mul")) return MUL;
    if (!strcmp (str, "div")) return DIV;
    if (!strcmp (str, "end")) return S_S;
    if (!strcmp (str, "set_addr")) return SET_RAM_ADDR;
    if (!strcmp (str, "get_addr")) return GET_RAM_ADDR;
    if (!strcmp (str, "ram_push")) return RAM_PUSH;
    if (!strcmp (str, "ram_pop")) return RAM_POP;
    return 0x00;
}

CMD_T Compilucter::is_cmd_if (char* str)
{
    if (!strcmp (str, "l0")) return L0;
    if (!strcmp (str, "l1")) return L1;
    if (!strcmp (str, "jmp")) return JMP;
    return 0x00;
}

REG_T Compilucter::is_reg (char* str)
{
    if (!strcmp (str, "X0")) return X0;
    if (!strcmp (str, "X1")) return X1;
    if (!strcmp (str, "X2")) return X2;
    if (!strcmp (str, "X3")) return X3;
    if (!strcmp (str, "X4")) return X4;
    if (!strcmp (str, "X5")) return X5;
    if (!strcmp (str, "X6")) return X6;
    if (!strcmp (str, "X7")) return X7;
    if (!strcmp (str, "X8")) return X8;
    if (!strcmp (str, "X9")) return X9;
    if (!strcmp (str, "XA")) return XA;
    if (!strcmp (str, "XB")) return XB;
    if (!strcmp (str, "XC")) return XC;
    if (!strcmp (str, "XD")) return XD;
    if (!strcmp (str, "XE")) return XE;
    if (!strcmp (str, "XF")) return XF;
    return 0x00;
}

int Compilucter::is_num (char *str, int64_t *num)
{
    *num = 0;
    bool is_negative = 0;
    if (str[0] == '-') {
        is_negative = 1;
        if (str[1] == '\0')
            return 0;
    }
    else {
        if (str[0] >= '0' && str[0] <= '9') {
            *num = *num * 10 + (str[0] - '0');
        }
        else
            return 0;
    }
    for (size_t i = 1; str[i] != '\0'; i++) {
        if (str[i] >= '0' && str[i] <= '9') {
            *num = *num * 10 + (str[i] - '0');
        }
        else
            return 0;
    }
    if (is_negative)
        *num *= -1;
    return 1;
}

char * Compilucter::find_line (char *target, size_t *num_line)
{
    *num_line = 1;
    char *the_line = this->code_orig;
    for (size_t i = 0; this->code + i < target; i++) {
        if (this->code_orig[i] == '\n') {
            *num_line += 1;
            the_line = this->code_orig + i + 1;
        }
    }
    return the_line;
}

//////////////////////////////////////////COMPILE////////////////////////////////////////////

int Compilucter::compile ()
{
    bool is_correct = 1;
    for (size_t i = 0; i < this->lexem_mas_size; i++) {
        if (this->lexem_mas[i].flag == INUM || this->lexem_mas[i].flag == FNUM) {
            this->error_handle (this->lexem_mas[i].origin, this->lexem_mas[i].origin, "Number must be an argument of some command.");
            is_correct = 0;
        }
        if (this->lexem_mas[i].flag == CMD) {
            switch (this->lexem_mas[i].cmd) {
            case PUSH :
                this->out_push (PUSH);
                i++;
                if (this->lexem_mas[i].flag != REG && this->lexem_mas[i].flag != INUM) {
                    this->error_handle (this->lexem_mas[i-1].origin, this->lexem_mas[i].origin, "\"push\" requires number or registry as an argument.");
                    is_correct = 0;
                }
                else {
                    if (this->lexem_mas[i].flag == INUM) {
                        this->out_push (INUM);
                        for (size_t j = 0; j < NUM_SIZE; j++)
                            this->out_push (this->lexem_mas[i].val.bin[j]);
                    }
                    if (this->lexem_mas[i].flag == REG) {
                        this->out_push (REG);
                        this->out_push (this->lexem_mas[i].reg);
                    }
                }
                break;
            case RAM_PUSH :
                this->out_push (RAM_PUSH);
                i++;
                if (this->lexem_mas[i].flag != REG && this->lexem_mas[i].flag != INUM) {
                    this->error_handle (this->lexem_mas[i-1].origin, this->lexem_mas[i].origin, "\"ram_push\" requires number or registry as an argument.");
                    is_correct = 0;
                }
                else {
                    if (this->lexem_mas[i].flag == INUM) {
                        this->out_push (INUM);
                        for (size_t j = 0; j < NUM_SIZE; j++)
                            this->out_push (this->lexem_mas[i].val.bin[j]);
                    }
                    if (this->lexem_mas[i].flag == REG) {
                        this->out_push (REG);
                        this->out_push (this->lexem_mas[i].reg);
                    }
                }
                break;
            case POP :
                this->out_push (POP);
                i++;
                if (this->lexem_mas[i].flag != REG) {
                    this->error_handle (this->lexem_mas[i-1].origin, this->lexem_mas[i].origin, "\"pop\" requires registry as an argument.");
                    is_correct = 0;
                }
                else {
                    this->out_push (this->lexem_mas[i].reg);
                }
                break;
            case RAM_POP :
                this->out_push (RAM_POP);
                i++;
                if (this->lexem_mas[i].flag != REG) {
                    this->error_handle (this->lexem_mas[i-1].origin, this->lexem_mas[i].origin, "\"ram_pop\" requires registry as an argument.");
                    is_correct = 0;
                }
                else {
                    this->out_push (this->lexem_mas[i].reg);
                }
                break;
            case IN :
                this->out_push (IN);
                i++;
                if (this->lexem_mas[i].flag != REG) {
                    this->error_handle (this->lexem_mas[i-1].origin, this->lexem_mas[i].origin, "\"in\" requires registry as an argument.");
                    is_correct = 0;
                }
                else {
                    this->out_push (this->lexem_mas[i].reg);
                }
                break;
            case OUT :
                this->out_push (OUT);
                i++;
                if (this->lexem_mas[i].flag != REG) {
                    this->error_handle (this->lexem_mas[i-1].origin, this->lexem_mas[i].origin, "\"out\" requires registry as an argument.");
                    is_correct = 0;
                }
                else {
                    this->out_push (this->lexem_mas[i].reg);
                }
                break;
            case SUM:
                this->out_push (SUM);
                break;
            case SUB:
                this->out_push (SUB);
                break;
            case MUL:
                this->out_push (MUL);
                break;
            case DIV:
                this->out_push (DIV);
                break;
            case S_S:
                this->out_push (S_S);
                break;
            case SET_RAM_ADDR:
                this->out_push (SET_RAM_ADDR);
                i++;
                if (this->lexem_mas[i].flag != REG && this->lexem_mas[i].flag != INUM) {
                    this->error_handle (this->lexem_mas[i-1].origin, this->lexem_mas[i].origin, "\"set_addr\" requires number or registry as an argument.");
                    is_correct = 0;
                }
                else {
                    if (this->lexem_mas[i].flag == INUM) {
                        this->out_push (INUM);
                        for (size_t j = 0; j < NUM_SIZE; j++)
                            this->out_push (this->lexem_mas[i].val.bin[j]);
                    }
                    if (this->lexem_mas[i].flag == REG) {
                        this->out_push (REG);
                        this->out_push (this->lexem_mas[i].reg);
                    }
                }
                break;
            case GET_RAM_ADDR:
                this->out_push (GET_RAM_ADDR);
                i++;
                if (this->lexem_mas[i].flag != REG) {
                    this->error_handle (this->lexem_mas[i-1].origin, this->lexem_mas[i].origin, "\"get-addr\" requires registry as an argument.");
                    is_correct = 0;
                }
                else {
                    this->out_push (this->lexem_mas[i].reg);
                }
                break;
            case L0:
                this->out_push (L0);
                this->labels_resolver[this->cur_labels_resolver].lex_num = i;
                this->labels_resolver[this->cur_labels_resolver].out_num = this->out_size; // kuda pishem adres
                this->cur_labels_resolver++;
                this->check_labels_resolver ();
                for (size_t j = 0; j < NUM_SIZE; j++)
                    this->out_push (0);
                break;
            case JMP:
                this->out_push (JMP);
                this->labels_resolver[this->cur_labels_resolver].lex_num = i;
                this->labels_resolver[this->cur_labels_resolver].out_num = this->out_size; // kuda pishem adres
                this->cur_labels_resolver++;
                this->check_labels_resolver ();
                for (size_t j = 0; j < NUM_SIZE; j++)
                    this->out_push (0);
                break;
            case L1:
                this->out_push (L1);
                this->labels_resolver[this->cur_labels_resolver].lex_num = i;
                this->labels_resolver[this->cur_labels_resolver].out_num = this->out_size; // kuda pishem adres
                this->cur_labels_resolver++;
                this->check_labels_resolver ();
                for (size_t j = 0; j < NUM_SIZE; j++)
                    this->out_push (0);
                break;
            default:
                std::cout << "What!?" << std::endl;
                is_correct = 0;
                break;
            }
        }
        if (this->lexem_mas[i].flag == LABEL) {
            size_t null_;
            if (!this->find_label (this->lexem_mas[i].label_name, &null_)) {
                this->error_handle (this->lexem_mas[i].origin, this->lexem_mas[i].label_name, "Conflicting declaration of label.");
                is_correct = 0;
                break;
            }
            if (this->is_cmd (this->lexem_mas[i].label_name)) {
                this->error_handle (this->lexem_mas[i].origin, this->lexem_mas[i].label_name, "Label name must not be equal to command name.");
                    is_correct = 0;
            }
            this->label_mas[this->cur_label].name = this->lexem_mas[i].label_name;
            this->label_mas[this->cur_label].place = this->out_size;
            this->cur_label++;
            this->check_label_mas ();
        }
    }
    for (size_t h = 0; h < this->cur_labels_resolver; h++) {
        size_t i = this->labels_resolver[h].lex_num;
        size_t out_n = this->labels_resolver[h].out_num;
        size_t place = 0;
        if (this->find_label (this->lexem_mas[i].label_name, &place)) {
            this->error_handle (this->lexem_mas[i].origin, this->lexem_mas[i].label_name, "Undefined reference to label.");
            is_correct = 0;
        }
        else {
            label_addr addr;
            addr.addr = place;
            for (size_t j = 0; j < sizeof (size_t); j++) {
                this->out_mas[out_n + j] = addr.bin[j];
            }
        }
    }
    this->out_push(S_S);
    if (is_correct)
        out_write ();
    return 0;
}

void Compilucter::out_push (uint8_t byte)
{
    this->out_mas[this->out_size] = byte;
    this->out_size++;
    this->check_out_mas();
}

void Compilucter::check_out_mas ()
{
    if (this->out_size >= this->out_capacity) {
        uint8_t *old = this->out_mas;
        this->out_capacity *= 2;
        this->out_mas = new uint8_t[this->out_capacity];
        memcpy (this->out_mas, old, this->out_size * sizeof (uint8_t));
        delete[] old;
        return;
    }
    return;
}

int Compilucter::out_write ()
{
    fwrite (this->out_mas, sizeof (uint8_t), this->out_size, this->out);
    return 0;
}

//////////////////////////////label/////////////////////////////////////////////

void Compilucter::check_label_mas ()
{
    if (this->cur_label >= this->label_mas_size) {
        struct label *old = this->label_mas;
        this->label_mas_size *= 2;
        this->label_mas = new struct label[this->label_mas_size];
        memcpy (this->label_mas, old, this->cur_label * sizeof (struct label));
        for (size_t i = this->cur_label; i < this->label_mas_size; i++) {
            this->label_mas[i] = {};
        }
        delete[] old;
        return;
    }
    return;
}

int Compilucter::find_label (char *name, size_t *place)
{
    *place = 0;
    for (size_t h = 0; h < this->cur_label; h++) {
        if (!strcmp(name, this->label_mas[h].name)) {
            *place = this->label_mas[h].place;
            return 0;
            break;
        }
    }
    return 1;
}

void Compilucter::check_labels_resolver()
{
    if (this->cur_labels_resolver >= this->labels_resolver_size) {
        struct label_res *old = this->labels_resolver;
        this->labels_resolver_size *= 2;
        this->labels_resolver = new struct label_res[this->labels_resolver_size];
        memcpy (this->labels_resolver, old, this->cur_labels_resolver * sizeof (struct label_res));
        for (size_t i = this->cur_labels_resolver; i < this->labels_resolver_size; i++) {
            this->labels_resolver[i] = {};
        }
        delete[] old;
        return;
    }
    return;
}
