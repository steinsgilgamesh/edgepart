#include <string.h>

#include "conversions.hpp"

// Removes \n from the end of line
void FIXLINE(char *s) {
    int len = (int)strlen(s) - 1;
    if (s[len] == '\n')
        s[len] = 0;
}

void convert_binary_elist(std::string inputfile, Converter *converter) {
    if (sizeof(vid_t) != sizeof(uint32_t)) { LOG(FATAL) << "Double check vid_t type!"; }
    FILE *inf = fopen(inputfile.c_str(), "rb");
    if (inf == NULL) {
        LOG(FATAL) << "Could not load:" << inputfile
                   << ", error: " << strerror(errno) << std::endl;
    }
    fseek(inf, 0L, SEEK_END);
    size_t file_size = ftell(inf);
    fseek(inf, 0L, SEEK_SET);
    size_t unit_size = sizeof(vid_t) * 2;
    size_t e_num = file_size / unit_size;
    LOG(INFO) << "Reading in binary edge list format, total edge number is [" << e_num << "]";
    uint32_t *buffer = (uint32_t*) malloc (unit_size * READ_BUF_SIZE);
    size_t bg_ofst = 0; size_t ed_ofst = READ_BUF_SIZE;
    if (ed_ofst > e_num) { ed_ofst = e_num; }
    while (ed_ofst != bg_ofst) {
        if ((size_t) ftell(inf) != unit_size * bg_ofst) {
            LOG(FATAL) << "Reading wrong offset : ftell / bg_ofst : "
                       << ftell(inf) / unit_size << " / " << bg_ofst;
        }
        LOG(INFO) << "Reading edge range no. [" << bg_ofst << ", " << ed_ofst << ")";
        size_t read_size = (ed_ofst - bg_ofst) * unit_size;
        if (!fread(buffer, 1, read_size, inf)) { LOG(FATAL) << "Reading ERROR!"; }
        for (size_t i = 0; i < ed_ofst - bg_ofst; ++i) {
            size_t j = i * 2;
            vid_t from = buffer[j]; vid_t to = buffer[j + 1];
            converter->add_edge(from, to);
        }
        bg_ofst = ed_ofst;
        ed_ofst += READ_BUF_SIZE;
        if (ed_ofst > e_num) { ed_ofst = e_num; }
    }

    free(buffer);
}


void convert_edgelist(std::string inputfile, Converter *converter) {
    FILE *inf = fopen(inputfile.c_str(), "r");
    size_t bytesread = 0;
    size_t linenum = 0;
    if (inf == NULL) {
        LOG(FATAL) << "Could not load:" << inputfile
                   << ", error: " << strerror(errno) << std::endl;
    }

    LOG(INFO) << "Reading in edge list format!" << std::endl;
    char s[1024];
    while (fgets(s, 1024, inf) != NULL) {
        linenum++;
        if (linenum % 10000000 == 0) {
            LOG(INFO) << "Read " << linenum << " lines, "
                      << bytesread / 1024 / 1024. << " MB" << std::endl;
        }
        FIXLINE(s);
        bytesread += strlen(s);
        if (s[0] == '#')
            continue; // Comment
        if (s[0] == '%')
            continue; // Comment

        char delims[] = "\t, ";
        char *t;
        t = strtok(s, delims);
        if (t == NULL) {
            LOG(FATAL) << "Input file is not in right format. "
                       << "Expecting \"<from>\t<to>\". "
                       << "Current line: \"" << s << "\"\n";
        }
        vid_t from = atoi(t);
        t = strtok(NULL, delims);
        if (t == NULL) {
            LOG(FATAL) << "Input file is not in right format. "
                       << "Expecting \"<from>\t<to>\". "
                       << "Current line: \"" << s << "\"\n";
        }
        vid_t to = atoi(t);

        if (from != to) {
            converter->add_edge(from, to);
        }
    }
    fclose(inf);
}

void convert_adjlist(std::string inputfile, Converter *converter) {
    FILE *inf = fopen(inputfile.c_str(), "r");
    if (inf == NULL) {
        LOG(FATAL) << "Could not load:" << inputfile
                   << " error: " << strerror(errno) << std::endl;
    }
    LOG(INFO) << "Reading in adjacency list format!" << std::endl;

    int maxlen = 1000000000;
    char *s = (char *)malloc(maxlen);

    size_t bytesread = 0;

    char delims[] = " \t";
    size_t linenum = 0;
    size_t lastlog = 0;

    while (fgets(s, maxlen, inf) != NULL) {
        linenum++;
        if (bytesread - lastlog >= 500000000) {
            LOG(INFO) << "Read " << linenum << " lines, "
                      << bytesread / 1024 / 1024. << " MB" << std::endl;
            lastlog = bytesread;
        }
        FIXLINE(s);
        bytesread += strlen(s);

        if (s[0] == '#')
            continue; // Comment
        if (s[0] == '%')
            continue; // Comment
        char *t = strtok(s, delims);
        vid_t from = atoi(t);
        t = strtok(NULL, delims);
        if (t != NULL) {
            vid_t num = atoi(t);
            vid_t i = 0;
            while ((t = strtok(NULL, delims)) != NULL) {
                vid_t to = atoi(t);
                if (from != to) {
                    converter->add_edge(from, to);
                }
                i++;
            }
            if (num != i)
                LOG(FATAL) << "Mismatch when reading adjacency list: " << num
                           << " != " << i << " s: " << std::string(s)
                           << " on line: " << linenum << std::endl;
        }
    }
    free(s);
    fclose(inf);
}

void convert(std::string basefilename, Converter *converter) {
    LOG(INFO) << "converting `" << basefilename << "'";
    if (basefilename.empty())
        LOG(FATAL) << "empty file name";
    if (converter->done()) {
        LOG(INFO) << "skip";
        return;
    }
    converter->init();
    if (FLAGS_filetype == "adjlist") {
        convert_adjlist(basefilename, converter);
    } else if (FLAGS_filetype == "edgelist") {
        convert_edgelist(basefilename, converter);
    } else if (FLAGS_filetype == "binary") {
        convert_binary_elist(basefilename, converter);
    } else { LOG(FATAL) << "unknown filetype"; }
    converter->finalize();
}

