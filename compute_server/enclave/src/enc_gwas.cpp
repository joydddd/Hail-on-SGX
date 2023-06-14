#include "enc_gwas.h"
#include "assert.h"
#include "float.h"

Row::Row(int _size, const std::vector<int>& sizes, int _num_dimensions, ImputePolicy _impute_policy) 
    : n(_size), impute_policy(_impute_policy), num_dimensions(_num_dimensions) {
    impute_average = impute_policy == ImputePolicy::Hail;
    //data.resize(_size);
    //data.push_back(new uint8_t[_size]);
    client_lengths.resize(sizes.size());
    for (int i = 0; i < sizes.size(); ++i) {
        client_lengths[i] = sizes[i];
    }

    it_count = 0;
    //read_row_len = n;
    read_row_len = ((n / 4) + (n % 4 == 0 ? 0 : 1));
}

void Row::reset() { 
    // loci = Loci();
    // alleles = Alleles();
}

int Row::read(const char line[]) {
    //std::cout << line << std::endl;
    static int count = 0;
    loci_str.clear();
    alleles_str.clear();
    int tabs_found = 0;
    int idx = 0;

    while(tabs_found < 2) {
        char curr_char = line[idx++];
        if (curr_char == '\t') {
            tabs_found++;
            continue;
        }
        if (tabs_found == 0) {
            loci_str.push_back(curr_char);
        } else {
            alleles_str.push_back(curr_char);
        }
    }
    try {
        loci = Loci(loci_str);
        if (loci.chrom_str == "X") {
            loci.chrom = LOCI_X;
        } else {
            loci.chrom = std::stoi(loci.chrom_str);
        }
        loci.loc = std::stoi(loci.loc_str);
        alleles.read(alleles_str);
    } catch (ReadtsvERROR &error) {
        std::cout << line << std::endl;
        throw ENC_ERROR("Invalid loci/alleles " + loci_str + "\t" + alleles_str
#ifdef DEBUG
                        + string("(loci)") + loci_str + " (alleles)" +
                        alleles_str
#endif
        );
    } catch (const std::invalid_argument& exception) {
        std::cout << "Invalid chrom/loc " << loci.chrom << " " << loci.loc << " " << std::endl;
        exit(0);
    }
    data = (uint8_t *)(line + loci_str.size() + alleles_str.size() + 2);

    return read_row_len + loci_str.size() + alleles_str.size() + 3;
}
void Row::combine(Row *other) {
    // /* check if loci & alleles match */
    // if (this->loci == Loci())
    //     this->loci = other->loci;
    // else if (other->loci != loci && other->loci != Loci())
    //     throw CombineERROR("locus mismatch");

    // if (alleles == Alleles())
    //     alleles = other->alleles;
    // else if (other->alleles != alleles && other->alleles != Alleles())
    //     throw CombineERROR("alleles mismatch");

    // this->n += other->n;
    // this->data.reserve(this->data.size() + other->data.size());
    // for (int i = 0; i < other->data.size(); i++) {
    //     data.push_back(other->data[i]);
    //     length.push_back(other->length[i]);
    // }
    // other->data.clear();
    // other->length.clear();
}

void Row::append_invalid_elts(int size) {
    // uint8_t *new_array = new uint8_t[size];
    // // data.push_back(new_array);
    // length.push_back(size);
    // n += size;
    // for (int i = 0; i < size; i++) new_array[i] = NA_uint8;
}

#ifdef DEBUG
#include <iostream>
void Row::print() {
    cout << loci << " " << alleles << endl;
    for (int j = 0; j < data.size(); j++) {
        for (int i = 0; i < length[j]; i++) {
            printf("%02x ", data[j][i]);
        }
        printf("\n");
    }
}
#endif


/////////////////////////////////////////////////////////
//////////////////////// Utility ////////////////////////
/////////////////////////////////////////////////////////

double read_entry_int(std::string &entry) {
    double ans;
    try {
        ans = std::stoi(entry);
    } catch (std::invalid_argument &error) {
        if (entry == "true" || entry == "True") {
            ans = 1;
        } else if (entry == "false" || entry == "False") {
            ans = 0;
        } else if (entry == "NA") {
            ans = nan("");
        } else {
            throw ReadtsvERROR("invalid int entry " + entry);
        }
    }
    return ans;
}

double bd_max(const std::vector<double>& vec) {
    double max = -DBL_MAX;
    for (int i = 0; i < vec.size(); i++) {
        double x = vec[i];
        if (x > max) max = x;
    }
    return max;
}

bool read_entry_bool(std::string& entry) {
    bool ans;
    try {
        int ans_int = stoi(entry);
        if (ans_int == 0) ans = false;
        if (ans_int == 1) ans = true;
    } catch (std::invalid_argument &error) {
        if (entry == "true" || entry == "True") {
            ans = true;
        } else if (entry == "false" || entry == "False") {
            ans = false;
        } else {
            throw ReadtsvERROR("invalid bool entry " + entry);
        }
    }
    return ans;
}

#ifdef DEBUG
void GWAS::print() const {
    cout << "y:";
    for (auto xx : y.data) cout << "\t" << xx;
    cout << endl;
}
#endif

/////////////////////////////////////////////////////////
////////////////   Covar    /////////////////////////////
/////////////////////////////////////////////////////////
int Covar::read(const char* input, int res_size) {
    std::vector<std::string> parts;
    if (res_size)
        parts.reserve(res_size + 1);

    split_delim(input, parts, '\t');

    if (!res_size) {
        res_size = parts.size() - 1;
        std::cout << "n size " << res_size << std::endl;
    }

    name_str = parts[0];

    int read_size = 0;
    for (int i = 1; i < res_size + 1; ++i) {
        data[i - 1][m] = std::stod(parts[i]);
        read_size++;
    }

    if (read_size != res_size || read_size != n) {
        std::cout << "Covar size mismatch" << std::endl;
    }
    m++;

    return read_size;
}

void Covar::reserve(int total_row_size) {
    data.reserve(total_row_size);
}

void Covar::init_1_covar(int total_row_size){
    if (total_row_size != n) {
        std::cout << "Covar size mismatch" << std::endl;
    }
    
    for (int i = 0; i < total_row_size; i++) {
        data[i][m] = 1;
    }

    m++;

}