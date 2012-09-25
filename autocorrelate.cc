//Autocorrelates the preamble against shifted versions of itself
//#include <cstdio>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdexcept>
#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <complex>
#include <iostream>
#include <fstream>
#include <string.h>

#include "wcdma_codes.h"

using namespace std;

// typedef std::complex<float> gr_complex;
#define DATA_TYPE gr_complex
#define syncode_length  512
// #define slot_length  5128
#define slot_length  5120
#define slots_per_frame 15
#define scrambling_code_group_number 64


float correlate(DATA_TYPE *buf, int code_ind);
int correlate_group(int *buf, int code_ind);
gr_complex getZcomplexConjugate(int frame_ind, int scrambling_group, int code_ind);
unsigned char gold_code(int n, int i);
unsigned char gold_code_x(int i);
unsigned char gold_code_y(int i);
void cpich_decode(int frame_ind, DATA_TYPE descrambled, DATA_TYPE *sum, DATA_TYPE *last_CPICH_symbol, DATA_TYPE *last_rotated_CPICH_symbol, float *lookup, float *angle);
void pccpch_decode(int frame_ind, DATA_TYPE descrambled, DATA_TYPE *sum, DATA_TYPE *last_PCCPCH_symbol, DATA_TYPE *last_rotated_PCCPCH_symbol, int *symbol_index);

// gr_complex d_preamble[fft_length];
// gr_complex garble[fft_length];

DATA_TYPE *d_data; //to store the data stream
DATA_TYPE *d_preamble = NULL;
DATA_TYPE *garble = NULL;
int desired_count;

unsigned char xy_scrambling_buffer[36 * 2][4];

float CPICHRotMatrix[4];
unsigned char PCCPCH_last_frame[270];

int main (int argc, const char *argv[])
{
    if (argc != 5)
    {
        fprintf (stderr, " usage: %s <preamble input file> <start index> <end index> <threshold>\n", argv[0]);
        exit(1);
    }
    int start_ind = atoi(argv[2]);
    int end_ind = atoi(argv[3]);
    int threshold = atoi(argv[4]);


    /*****
    ** read raw data
    *****/
    FILE *d_fp;
    if ((d_fp = fopen(argv[1], "rb")) == NULL)
    {
        fprintf(stderr, "data file cannot be opened\n");
        assert(false);
    }

    //get file size
    fseek( d_fp, 0L, SEEK_END );
    long endPos = ftell( d_fp );
    fclose(d_fp);

    DATA_TYPE *tmp_d_data = (DATA_TYPE *) malloc(endPos);

    //re-open file
    if ((d_fp = fopen(argv[1], "rb")) == NULL)
    {
        fprintf(stderr, "data file cannot be opened\n");
        assert(false);
    }

    // int count = fread_unlocked(tmp_d_data, sizeof(DATA_TYPE), endPos / sizeof(DATA_TYPE), d_fp);
    fread_unlocked(tmp_d_data, sizeof(DATA_TYPE), endPos / sizeof(DATA_TYPE), d_fp);
    desired_count = end_ind - start_ind;
    d_data = (DATA_TYPE *) malloc(sizeof(DATA_TYPE) * desired_count);
    memcpy(d_data, tmp_d_data + start_ind, sizeof(DATA_TYPE) * desired_count);


    /*****
    ** find primary sync code
    *****/
    int pre_syncode_ind = 0;
    int first_slot_ind = -1;
    for (int i = 0 ; i < desired_count - syncode_length; i++)
    {
        int offset = i;

        float coeff = correlate(&(d_data[i]), 0);

        if (coeff > threshold)
        {
            cout << offset << "\t" << coeff << "\t" << (offset - pre_syncode_ind) << endl;
            pre_syncode_ind = offset;
            if (first_slot_ind < 0)
            {
                first_slot_ind = offset;
            }
        }
    }
    // cheat for first index
    first_slot_ind = 19;


    /*****
    ** verify PSC
    *****/
    // cout << endl << "start:" << endl;
    // d_preamble = (DATA_TYPE *) malloc(syncode_length * sizeof(DATA_TYPE));
    // // memcpy(d_preamble, d_data + 5147, syncode_length * sizeof(DATA_TYPE));
    // memcpy(d_preamble, sync_codes[0], syncode_length * sizeof(DATA_TYPE));

    // for (int i = 0; i < desired_count - syncode_length; i ++)
    // {
    //     gr_complex sum = 0;
    //     //inner-product
    //     for (int j = 0 ; j < syncode_length; j ++)
    //     {
    //         sum += (d_data[i + j] * conj(d_preamble[j]));
    //     }
    //     cout << i << " " <<  std::abs(sum) << "\n";
    // }


    /*****
    ** find Secondary Sync Code
    ** 1. find code index for 15 slots
    *****/
    cout << endl << "find Secondary Sync Code:" << endl;
    // for all slots
    int slot_cnt = 0;
    int *code_vector = (int *)malloc(sizeof(int) * slots_per_frame);
    for (int i = first_slot_ind; i < desired_count - syncode_length; i += slot_length)
    {
        slot_cnt ++;
        cout << "> slot " << slot_cnt << endl;
        float max_coeff = -1;
        int max_code_ind;
        // for all possible codes
        for (int j = 1; j <= 16; j ++)
        {
            float coeff = correlate(d_data + i, j);
            cout << "  " << j << "=" << coeff << ",";
            if (coeff > max_coeff)
            {
                max_coeff = coeff;
                max_code_ind = j;
            }
        }
        cout << endl << "  > max: " << max_code_ind << "=" << max_coeff << endl;
        code_vector[slot_cnt - 1] = max_code_ind;

        if (slot_cnt == slots_per_frame)
        {
            break;
        }
        assert(slot_cnt < slots_per_frame);
    }


    /*****
    ** 2. find code group
    *****/
    cout << endl << "find code group:" << endl;
    int max_group_coeff = -1;
    int max_group_code_ind;
    int max_group_offset;
    for (int offset = 0; offset < slots_per_frame; offset ++)
    {
        int *garble_code_vector = (int *)malloc(sizeof(int) * slots_per_frame);

        for (int i = 0; i < slots_per_frame; i ++)
        {
            // garble_code_vector[(i + offset) % slots_per_frame] = code_vector[i];
            garble_code_vector[i] = code_vector[(i + offset) % slots_per_frame];
            // cout << garble_code_vector[(i + offset) % slots_per_frame] << " ";
            cout << garble_code_vector[i] << " ";
        }
        cout << endl;

        for (int i = 0; i < scrambling_code_group_number; i ++)
        {
            int group_coeff = correlate_group(garble_code_vector, i);
            cout << " " << i << "=" << group_coeff << ",";

            if (group_coeff > max_group_coeff)
            {
                max_group_coeff = group_coeff;
                max_group_code_ind = i;
                max_group_offset = offset;
            }
        }
        cout << endl;
    }
    cout << "--> max: " << max_group_code_ind << "=" << max_group_coeff << "," << max_group_offset << endl;



    /*****
    ** plot frame
    *****/
    int frame_start_ind = first_slot_ind + slot_length * max_group_offset;
    ofstream f_frame;
    f_frame.open ("sync_frame.txt");
    for (int i = 0; i < slots_per_frame * slot_length * 2; i ++)
    {
        f_frame << i << " " << abs(d_data[frame_start_ind + i]) << " " << arg(d_data[frame_start_ind + i]) << " " << d_data[frame_start_ind + i].real() << " " << d_data[frame_start_ind + i].imag() << endl;
    }
    f_frame.close();



    /*****
    ** DEBUG
    *****/
    // int frame_start_ind = first_slot_ind + slot_length * max_group_offset;
    // for (int i = 0; i < 10; i ++) {
    //     int index = frame_start_ind + i * slot_length * slots_per_frame;
    //     cout << index << endl;
    // }
    /*****
    ** DEBUG 2
    *****/
    // int fi = 0;
    // int sg = 0;
    // int psci = 0; 
    // int code_n = (sg * 8 + psci) * 16;
    // DATA_TYPE tmpz1 = getZcomplexConjugate(fi, sg, psci);
    // unsigned char tmpz2 = gold_code(code_n, fi);
    // cout << tmpz1 << ", " << ((int)tmpz2) << endl;

    // fi = 18;
    // sg = 0;
    // psci = 0; 
    // code_n = (sg * 8 + psci) * 16;
    // tmpz1 = getZcomplexConjugate(fi, sg, psci);
    // tmpz2 = gold_code(code_n, fi);
    // cout << tmpz1 << ", " << ((int)tmpz2) << endl;

    // fi = 2;
    // sg = 3;
    // psci = 4; 
    // code_n = (sg * 8 + psci) * 16;
    // tmpz1 = getZcomplexConjugate(fi, sg, psci);
    // tmpz2 = gold_code(code_n, fi);
    // cout << tmpz1 << ", " << ((int)tmpz2) << endl;
    // return 0;



    /*****
    ** find primary scrambling code
    *****/
    // cheat: frame index
    int frame_boundry_ind[10] = {5147, 82070, 158993, 235916, 312839, 389762, 466685, 543608, 620523, 697455};
    // int frame_start_ind = first_slot_ind + slot_length * max_group_offset;
    int num_frames_psc = 10; // 80;
    float acc_res_total[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    int max_psc_ind_total;
    for (int frame_cnt = 0; frame_cnt < num_frames_psc; frame_cnt ++)
    {
        float acc_res[8] = {0, 0, 0, 0, 0, 0, 0, 0};

        for (int psc_ind = 0; psc_ind < 8; psc_ind ++)
        {
            DATA_TYPE sum = 0;

            for (int frame_ind = 0; frame_ind < slots_per_frame * slot_length; frame_ind ++)
            {
                // int index = frame_start_ind + frame_cnt * slot_length * slots_per_frame + frame_ind;
                int index = frame_boundry_ind[frame_cnt] + frame_ind;

                DATA_TYPE z = getZcomplexConjugate(frame_ind, max_group_code_ind, psc_ind);
                DATA_TYPE descrambled = d_data[index] * z;
                sum += descrambled;
            }
            acc_res[psc_ind] += std::abs(sum);
            acc_res_total[psc_ind] += std::abs(sum);
        }

        float max_psc_sum = -1;
        int max_psc_ind;
        float max_psc_sum_total = -1;
        for (int i = 0; i < 8; i ++)
        {
            cout << i << "=" << acc_res[i] << ",";
            if (max_psc_sum < acc_res[i])
            {
                max_psc_ind = i;
                max_psc_sum = acc_res[i];
            }
            if (max_psc_sum_total < acc_res_total[i])
            {
                max_psc_ind_total = i;
                max_psc_sum_total = acc_res_total[i];
            }
        }
        cout << endl << "> frame " << frame_cnt << ": " << max_psc_ind << " / " << max_psc_ind_total << endl;
    }


    /*****
    ** DEBUG
    *****/
    DATA_TYPE psc[76800];
    assert(slots_per_frame * slot_length == 76800);
    for (int frame_ind = 0; frame_ind < slots_per_frame * slot_length; frame_ind ++) {
        psc[frame_ind] = getZcomplexConjugate(frame_ind, max_group_code_ind, max_psc_ind_total);
        // psc[frame_ind] = getZcomplexConjugate(frame_ind, max_group_code_ind, 1);
    }
    num_frames_psc = 5; // 80;
    cout << endl << "DEBUG: correlation of PSC" << endl;
    ofstream tmp_file;
    tmp_file.open("DEBUG_corref_PSC.txt");
    for (int frame_cnt = 0; frame_cnt < num_frames_psc; frame_cnt ++) {
        for (int frame_ind = 0; frame_ind < slots_per_frame * slot_length - 512; frame_ind ++) {
            DATA_TYPE tmp_sum = 0;
            for(int i = 0; i < 512; i ++ ) {
                int index = frame_boundry_ind[frame_cnt] + frame_ind + i;
                DATA_TYPE z = psc[frame_ind + i];
                // DATA_TYPE z = psc[i];
                DATA_TYPE descrambled = d_data[index] * z;
                tmp_sum += descrambled;
            }
            tmp_file << frame_cnt << " " << std::abs(tmp_sum) << endl;
        }
    }
    tmp_file.close();


    /*****
    ** decode Common Pilot Channel
    *****/
    // int frame_start_ind = first_slot_ind + slot_length * max_group_offset;
    num_frames_psc = 10; // 80;
    DATA_TYPE last_CPICH_symbol = 0;
    DATA_TYPE last_rotated_CPICH_symbol;
    DATA_TYPE last_PCCPCH_symbol;
    DATA_TYPE last_rotated_PCCPCH_symbol;
    DATA_TYPE cpich_sum = 0;
    DATA_TYPE pccpch_sum = 0;
    float lookup = 1;
    float angle;
    int symbol_index = 0;

    ofstream f_cpich;
    f_cpich.open("sync_cpich.txt");
    ofstream f_pccpch;
    f_pccpch.open("sync_pccpch.txt");
    
    for (int frame_cnt = 0; frame_cnt < num_frames_psc; frame_cnt ++)
    {
        for (int frame_ind = 0; frame_ind < slots_per_frame * slot_length; frame_ind ++)
        {
            // int index = frame_start_ind + frame_cnt * slot_length * slots_per_frame + frame_ind;
            int index = frame_boundry_ind[frame_cnt] + frame_ind;
            DATA_TYPE z = getZcomplexConjugate(frame_ind, max_group_code_ind, max_psc_ind_total);
            DATA_TYPE descrambled = d_data[index] * z;

            cpich_decode(frame_ind, descrambled, &cpich_sum, &last_CPICH_symbol, &last_rotated_CPICH_symbol, &lookup, &angle);
            pccpch_decode(frame_ind, descrambled, &pccpch_sum, &last_PCCPCH_symbol, &last_rotated_PCCPCH_symbol, &symbol_index);

            if((frame_ind % 512) == 511) {
                f_cpich << last_CPICH_symbol.real() << " " << last_CPICH_symbol.imag() << " " << last_rotated_CPICH_symbol.real() << " " << last_rotated_CPICH_symbol.imag() << " " << angle << endl;

                f_pccpch << last_PCCPCH_symbol.real() << " " << last_PCCPCH_symbol.imag() << " " << last_rotated_PCCPCH_symbol.real() << " " << last_rotated_PCCPCH_symbol.imag() << endl;
            }
        }
    }

    f_cpich.close();
    f_pccpch.close();





    return 0;
}

float correlate(DATA_TYPE *buf, int code_ind)
{
    gr_complex sum = 0;

    for (int i = 0 ; i < syncode_length; i++)
    {
        // sum += (buf[i] * conj(sync_codes[code_ind][i]));
        sum += (buf[i] * sync_codes[code_ind][i]);
    }

    return std::abs(sum);
}

int correlate_group(int *buf, int code_ind)
{
    int sum = 0;

    for (int i = 0 ; i < slots_per_frame; i++)
    {
        if (buf[i] == scrambling_code_group_table[code_ind][i])
        {
            sum ++;
        }
    }

    return sum;
}

gr_complex getZcomplexConjugate(int frame_ind, int scrambling_group, int code_ind)
{
    if (frame_ind == 0)
    {
        // copy initial sequence
        for (int i = 0; i < 36; i++)
        {
            for (int j = 0; j < 4; j++)
            {
                xy_scrambling_buffer[i][j] = scrambling_codes[scrambling_group * 8 + code_ind][i][j];
            }
        }
    }

    int index = frame_ind % 36; //+2;

    // calculate next value in next sequence
    for (int i = 0; i < 2; i++)
    {
        xy_scrambling_buffer[index + 36][i * 2] =
            (xy_scrambling_buffer[index + 14][i * 2] +
             xy_scrambling_buffer[index][i * 2]) % 2;

        xy_scrambling_buffer[index + 36][1 + i * 2] =
            (xy_scrambling_buffer[index + 20][1 + i * 2] +
             xy_scrambling_buffer[index + 14][1 + i * 2] +
             xy_scrambling_buffer[index + 10][1 + i * 2] +
             xy_scrambling_buffer[index][1 + i * 2]) % 2;
    }

    int z1 = (xy_scrambling_buffer[index][0] + xy_scrambling_buffer[index][1]) % 2;
    int z2 = (xy_scrambling_buffer[index][2] + xy_scrambling_buffer[index][3]) % 2;
    float re = (z1 == 1) ? -1 : 1;
    float im = (z2 == 1) ? -1 : 1;

    // copy newly calculated value downto low part
    xy_scrambling_buffer[index][0] = xy_scrambling_buffer[index + 36][0];
    xy_scrambling_buffer[index][1] = xy_scrambling_buffer[index + 36][1];
    xy_scrambling_buffer[index][2] = xy_scrambling_buffer[index + 36][2];
    xy_scrambling_buffer[index][3] = xy_scrambling_buffer[index + 36][3];

    return gr_complex(re, -im);
}


unsigned char gold_code(int n, int i)
{
    // zn(i) = x((i+n) modulo 218 - 2) + y(i)
    // x(i+18) =x(i+7) + x(i) modulo 2, i=0,...,218-20,
    // y(i+18) = y(i+10)+y(i+7)+y(i+5)+y(i) modulo 2, i=0,..., 218-20.
    return (gold_code_x(((i+n) % 262144) - 2) + gold_code_y(i)) % 2;
}

unsigned char gold_code_x(int i)
{
    unsigned char x_series[36] = {1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    
    if(i < 36) {
        // cout << "x=" << x_series[i] << endl;
        return x_series[i];
    }
    else {
        return (gold_code_x(i-22) + gold_code_x(i-36)) % 2;
    }
}

unsigned char gold_code_y(int i)
{
    unsigned char y_series[36] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
    
    if(i < 36) {
        // cout << "y=" << ((int)y_series[i]) << endl;
        return y_series[i];
    }
    else {
        return (gold_code_y(i-16) + gold_code_y(i-22) + gold_code_y(i-26) + gold_code_y(i-36)) % 2;
    }
}


void cpich_decode(int frame_ind, DATA_TYPE descrambled, DATA_TYPE *sum, DATA_TYPE *last_CPICH_symbol, DATA_TYPE *last_rotated_CPICH_symbol, float *lookup, float *angle)
{
    if (frame_ind % 512 == 0)
    {
        *sum = gr_complex(0.0f, 0.0f);
    }

    *sum += descrambled;// * gr_complex(1.0f,1.0f); // * gr_complex(1.0f,-1.0f);

    if (frame_ind % 512 == 511)
    {
        float alpha = 1.0f;

        *last_CPICH_symbol = *sum * alpha + *last_CPICH_symbol * (1 - alpha);
        // simplified cross product in order to find directiong of rotation from 1+1j
        // 0 = online(no rotation), + = counter clockwise, - = clockwise
        float dir = last_CPICH_symbol->real() - last_CPICH_symbol->imag();

        float l1 = std::abs(*last_CPICH_symbol);
        float l2 = 1.414213562f; // sqrt(2);

        // simplified dot product against 1+1j
        float dot = last_CPICH_symbol->real() + last_CPICH_symbol->imag();

        if (dir > 0)
        {
            dir = 1;
        }
        else
        {
            dir = -1;
        }

        float l3 = l1 * l2;

        // static float lookup = 1.0f;

        if (l3 != 0 && !isnan(l3))
        {
            *lookup = ((dot / l1) / l2);
        }

        *angle = acos(*lookup) * dir;
        //angle-=3.1415/4;
        //if (data->frame_cntr%512==0) {std::cout << "Angle: " << angle << " dot: " << dot<< " lookup: " << lookup;}
        float m[4];
        m[0] = cos(*angle);
        m[1] = -sin(*angle);
        m[2] = sin(*angle);
        m[3] = cos(*angle);

        DATA_TYPE a = gr_complex(
                          m[0] * last_CPICH_symbol->real() + m[1] * last_CPICH_symbol->imag(),
                          m[2] * last_CPICH_symbol->real() + m[3] * last_CPICH_symbol->imag()
                      );

        *last_rotated_CPICH_symbol = a;
        //data->last_rotated_CPICH_symbol = data->last_CPICH_symbol;
        //      std::cout << data->last_rotated_CPICH_symbol.real() << ", "
        //              << data->last_rotated_CPICH_symbol.imag() << ". ";
        // copy phase rotation matrix
        CPICHRotMatrix[0] = m[0];
        CPICHRotMatrix[1] = m[1];
        CPICHRotMatrix[2] = m[2];
        CPICHRotMatrix[3] = m[3];

    }
}


void pccpch_decode(int frame_ind, DATA_TYPE descrambled, DATA_TYPE *sum, DATA_TYPE *last_PCCPCH_symbol, DATA_TYPE *last_rotated_PCCPCH_symbol, int *symbol_index)
{
    
    // no data on PCCPCH on the first 512 half chips
    if (frame_ind % 5120 < 512)
    {
        return;
    }

    int part = (frame_ind % 512);

    if (part == 0)
    {
        *sum = gr_complex(0.0f, 0.0f);
    }

    // apply channelisation code 256,1
    if (part < 256)
    {
        *sum += descrambled;
    }
    else
    {
        *sum += gr_complex(descrambled * -1.0f);
    }

    // entire symbol decoded phase rotate and save
    if (part == 511)
    {
        *last_PCCPCH_symbol = *sum;

        *last_rotated_PCCPCH_symbol = gr_complex(
                                         CPICHRotMatrix[0] * last_PCCPCH_symbol->real() + CPICHRotMatrix[1] * last_PCCPCH_symbol->imag(),
                                         CPICHRotMatrix[2] * last_PCCPCH_symbol->real() + CPICHRotMatrix[3] * last_PCCPCH_symbol->imag()
                                     );

        //data->last_rotated_PCCPCH_symbol = data->last_PCCPCH_symbol;

        int b1, b2;

        b1 = last_rotated_PCCPCH_symbol->real() < 0.0f;
        b2 = last_rotated_PCCPCH_symbol->imag() < 0.0f;

        // Should be as this
        //          b1 = data->last_rotated_PCCPCH_symbol.real() < 0.0f;
        //          b2 = data->last_rotated_PCCPCH_symbol.imag() < 0.0f;

        PCCPCH_last_frame[*symbol_index * 2] = b1;
        PCCPCH_last_frame[*symbol_index * 2 + 1] = b2;

        // if (data->PCCPCH_frame_buffer_counter < PCCPCH_FRAME_BUFFER_SIZE)
        // {
        //     // write data
        //     (*data->PCCPCH_frame_buffer)[data->PCCPCH_frame_buffer_counter * 270 + symbol_index * 2] = b1;
        //     (*data->PCCPCH_frame_buffer)[data->PCCPCH_frame_buffer_counter * 270 + symbol_index * 2 + 1] = b2;
        // }
        // else
        // {
        //     //std::cout << "PCCPCH_frame_buffer_counter: " << data->PCCPCH_frame_buffer_counter << std::endl;
        //     *(data->done) = true;
        // }

        (*symbol_index) ++;
        if (*symbol_index >= 135)
        {
            // increase index
            // cout << "\nPCCPCH data: ";
            // for (int i = 0; i < 270; i++) {
            //     cout << (int) PCCPCH_last_frame[i];
            // }
            // cout << endl;

            symbol_index = 0;
        }

    }
    //half chip in slot
    //data->frame_sample_index%5120;


}