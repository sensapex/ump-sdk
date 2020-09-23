/*
 * A sample C-program for Sensapex micromanipulator SDK (umpsdk)
 *
 * Copyright (c) 2016-2020, Sensapex Oy
 * All rights reserved.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/timeb.h>
#include <stdint.h>
#include "libump.h"

#define VERSION_STR   "v0.121"
#define COPYRIGHT "Copyright (c) Sensapex. All rights reserved"

#define DEV     1
#define UPDATE  200

typedef struct params_s
{
    int x, y, z, w, X, Y, Z, W;
    int verbose, update, loop, dev, speed;
    char *address;
} params_struct;

void usage(char **argv)
{
    fprintf(stderr,"usage: %s [opts]\n",argv[0]);
    fprintf(stderr,"Generic options\n");
    fprintf(stderr,"-d\tdev (def: %d)\n", DEV);
    fprintf(stderr,"-v\tverbose\n");
    fprintf(stderr,"-a\taddress (def: %s)\n", LIBUMP_DEF_BCAST_ADDRESS);
    fprintf(stderr,"Position change\n");
    fprintf(stderr,"-x\trelative target (um, decimal value accepted, negative value for backward)\n");
    fprintf(stderr,"-y\trelative target \n");
    fprintf(stderr,"-z\trelative target \n");
    fprintf(stderr,"-w\trelative target \n");
    fprintf(stderr,"-X\tabs target (um, decimal value accepted\n");
    fprintf(stderr,"-Y\tabs target \n");
    fprintf(stderr,"-Z\tabs target \n");
    fprintf(stderr,"-W\tabs target \n");
    fprintf(stderr,"-n\tcount\tloop between current and target positions or take multiple relative steps into same direction\n");
    exit(1);
}

// Exits via usage() if an error occurs
void parse_args(int argc, char *argv[], params_struct *params)
{
    int i, v;
    float f;
    memset(params, 0, sizeof(params_struct));
    params->X = LIBUMP_ARG_UNDEF;
    params->Y = LIBUMP_ARG_UNDEF;
    params->Z = LIBUMP_ARG_UNDEF;
    params->W = LIBUMP_ARG_UNDEF;
    params->dev = DEV;
    params->update = UPDATE;
    params->address = LIBUMP_DEF_BCAST_ADDRESS;
    for(i = 1; i < argc; i++)
    {
        if(argv[i][0] == '-')
        {
            switch(argv[i][1])
            {
            case 'h': 
                usage(argv);
                break;
            case 'v':
                params->verbose++;
                break;
            case '1':
                params->verbose = 0;
                break;
            case 'n':
                if(i < argc-1 && sscanf(argv[++i],"%d",&v) == 1 && v > 0)
                    params->loop = v;
                else
                    usage(argv);
                break;
            case 'u':
                if(i < argc-1 && sscanf(argv[++i],"%d",&v) == 1 && v > 0)
                    params->update = v;
                else
                    usage(argv);
                break;
            case 'd':
                if(i < argc-1 && sscanf(argv[++i],"%d",&v) == 1 && v > 0)
                    params->dev = v;
                else
                    usage(argv);
                break;
            case 's':
                if(i < argc-1 && sscanf(argv[++i],"%d",&v) == 1 && v > 0)
                    params->speed = v;
                else
                    usage(argv);
                break;
            case 'x':
                if(i < argc-1 && sscanf(argv[++i],"%f",&f) == 1)
                    params->x = (int)(f*1000.0);
                else
                    usage(argv);
                break;
            case 'y':
                if(i < argc-1 && sscanf(argv[++i],"%f",&f) == 1)
                    params->y = (int)(f*1000.0);
                else
                    usage(argv);
                break;
            case 'z':
                if(i < argc-1 && sscanf(argv[++i],"%f",&f) == 1)
                    params->z = (int)(f*1000.0);
                else
                    usage(argv);
                break;
            case 'w':
                if(i < argc-1 && sscanf(argv[++i],"%f",&f) == 1)
                    params->w = (int)(f*1000.0);
                else
                    usage(argv);
                break;
            case 'X':
                if(i < argc-1 && sscanf(argv[++i],"%f",&f) == 1 && f >= 0)
                    params->X = (int)(f*1000.0);
                else
                    usage(argv);
                break;
            case 'Y':
                if(i < argc-1 && sscanf(argv[++i],"%f",&f) == 1 && f >= 0)
                    params->Y = (int)(f*1000.0);
                else
                    usage(argv);
                break;
            case 'Z':
                if(i < argc-1 && sscanf(argv[++i],"%f",&f) == 1 && f >= 0)
                    params->Z = (int)(f*1000.0);
                else
                    usage(argv);
                break;
            case 'W':
                if(i < argc-1 && sscanf(argv[++i],"%f",&f) == 1 && f >= 0)
                    params->W = (int)(f*1000.0);
                else
                    usage(argv);
                break;
            case 'a':
                if(i < argc-1 && argv[i+1][0] != '-')
                    params->address = argv[++i];
                else
                    usage(argv);
                break;
            default:
                usage(argv);
                break;
            }
        }
        else
            usage(argv);
    }
}

static float um(const int nm)
{
    if(nm == LIBUMP_ARG_UNDEF)
        return 0.0;
    return (float)nm/1000.0;
}

int main(int argc, char *argv[])
{
    ump_state *handle = NULL;
    int ret, status, loop = 0;
    int home_x = 0, home_y = 0, home_z = 0, home_w = 0;
    params_struct params;

    parse_args(argc, argv, &params);

    if((handle = ump_open(params.address, LIBUMP_DEF_TIMEOUT, LIBUMP_DEF_GROUP)) == NULL)
    {
        // Feeding NULL handle is intentional, it obtains the
        // last OS error which prevented the port to be opened
        fprintf(stderr, "Open failed - %s\n", ump_last_errorstr(handle));
        exit(1);
    }

    if(ump_select_dev(handle, params.dev) <0)
    {
        fprintf(stderr, "Select dev failed - %s\n", ump_last_errorstr(handle));
        ump_close(handle);
        exit(2);
    }

    if(ump_get_positions(handle, &home_x, &home_y, &home_z, &home_w) < 0)
        fprintf(stderr, "Get positions failed - %s\n", ump_last_errorstr(handle));
    else
        printf("Current position: %3.2f %3.2f %3.2f %3.2f\n", um(home_x), um(home_y), um(home_z), um(home_w));

    do
    {
        int x, y, z, w;

        if(params.X != LIBUMP_ARG_UNDEF)
            x = (loop&1)?home_x:params.X;
        else if(params.x)
            x = home_x + (loop+1)*params.x;
        else
            x = LIBUMP_ARG_UNDEF;

        if(params.Y != LIBUMP_ARG_UNDEF)
            y = (loop&1)?home_y:params.Y;
        else if(params.y)
            y = home_y + (loop+1)*params.y;
        else
            y = LIBUMP_ARG_UNDEF;

        if(params.Z != LIBUMP_ARG_UNDEF)
            z = (loop&1)?home_z:params.Z;
        else if(params.z)
            z = home_z + (loop+1)*params.z;
        else
            z = LIBUMP_ARG_UNDEF;

        if(params.W != LIBUMP_ARG_UNDEF)
            w = (loop&1)?home_w:params.W;
        else if(params.w)
            w = home_w + (loop+1)*params.w;
        else
            w = LIBUMP_ARG_UNDEF;

        if(params.loop)
            printf("Target position: %3.2f %3.2f %3.2f %3.2f (%d/%d)\n", um(x), um(y), um(z), um(w), loop+1, params.loop);
        else
            printf("Target position: %3.2f %3.2f %3.2f %3.2f\n", um(x), um(y), um(z), um(w));

        if((ret = ump_goto_position(handle, x, y, z, w, params.speed)) < 0)
        {
            fprintf(stderr, "Goto position failed - %s\n", ump_last_errorstr(handle));
            continue;
        }
        if(!params.loop && !params.verbose)
            break;
        do
        {
            ump_receive(handle, params.update);
            status = ump_get_drive_status(handle);
            if(params.verbose)
            {
                if(status < 0)
                    fprintf(stderr, "Status read failed - %s\n", ump_last_errorstr(handle));
                else if(ump_get_positions(handle, &x, &y, &z, &w) < 0)
                    fprintf(stderr, "Get positions failed - %s\n", ump_last_errorstr(handle));
                else
                    printf("%3.2f %3.2f %3.2f %3.2f status %02X\n", um(x), um(y), um(z), um(w), status);
            }
        }
        while(status == LIBUMP_POS_DRIVE_BUSY);
	} while(++loop < params.loop);
    ump_close(handle);
    exit(!ret);
}
