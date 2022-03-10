#include "enc_gwas.h"
#include "assert.h"

Row::Row(size_t _size) : n(_size) {
    data.push_back(new uint8_t[_size]);
    length.push_back(_size);
}

void Row::reset() { 
    loci = Loci();
    alleles = Alleles();
}

size_t Row::read(const char line[]) {
    std::vector<std::string> parts;
    if (split_delim(line, parts, '\t', 2) != 2) {
        throw ENC_ERROR("Invalid row parse with " + std::to_string(parts.size()) + " args\n");
    }
    const std::string& loci_str = parts.front();
    const std::string& alleles_str = parts.back();
    try {
        loci = Loci(loci_str);
        if (loci.chrom_str == "X")
            loci.chrom = LOCI_X;
        else
            loci.chrom = stoi(loci.chrom_str);
        loci.loc = stoi(loci.loc_str);
        alleles.read(alleles_str);
    } catch (ReadtsvERROR &error) {
        std::cout << line << std::endl;
        throw ENC_ERROR("Invalid loci/alleles " + loci_str + "\t" + alleles_str
#ifdef DEBUG
                        + string("(loci)") + loci_str + " (alleles)" +
                        alleles_str
#endif
        );
    }
    for (size_t i = 0; i < n; i++) {
        data[0][i] =
            (uint8_t)line[i + loci_str.size() + alleles_str.size() + 2] - uint8_OFFSET;
        if (data[0][i] > NA_uint8){
            throw ReadtsvERROR("Invalid row entry for " + loci_str + "\t" + alleles_str);
        }
    }
    if (line[n + loci_str.size() + alleles_str.size() + 2] != '\n')
        throw ReadtsvERROR("Invalid row terminator");
    return n + loci_str.size() + alleles_str.size() + 3;
}
void Row::combine(Row *other) {
    /* check if loci & alleles match */
    if (this->loci == Loci())
        this->loci = other->loci;
    else if (other->loci != loci && other->loci != Loci())
        throw CombineERROR("locus mismatch");

    if (alleles == Alleles())
        alleles = other->alleles;
    else if (other->alleles != alleles && other->alleles != Alleles())
        throw CombineERROR("alleles mismatch");

    this->n += other->n;
    this->data.reserve(this->data.size() + other->data.size());
    for (size_t i = 0; i < other->data.size(); i++) {
        data.push_back(other->data[i]);
        length.push_back(other->length[i]);
    }
    other->data.clear();
    other->length.clear();
}

void Row::append_invalid_elts(size_t size) {
    uint8_t *new_array = new uint8_t[size];
    data.push_back(new_array);
    length.push_back(size);
    n += size;
    for (size_t i = 0; i < size; i++) new_array[i] = NA_uint8;
}

#ifdef DEBUG
#include <iostream>
void Row::print() {
    cout << loci << " " << alleles << endl;
    for (size_t j = 0; j < data.size(); j++) {
        for (size_t i = 0; i < length[j]; i++) {
            printf("%02x ", data[j][i]);
        }
        printf("\n");
    }
}
#endif