/* Copyright 2015-2017 - Nico Curti, Alessandro Fabbri */

/***************************************************************************
This file is part of srsa.
srsa is free software : you can redistribute it and / or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
srsa is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
GNU General Public License for more details.
You should have received a copy of the GNU General Public License
along with srsa. If not, see <http://www.gnu.org/licenses/>.
***************************************************************************/

#include "rsa.hpp"

using namespace std;

typedef unsigned long int Uint;

void usage(const char* progname)
{
	cout << "Usage : " << progname << " [MODE] [ARGS] [KEY]" << endl << endl
		<< "MODE :\tg  = generate key pair\nARGS :\tp q (prime integers)\nKEY  :\tkey pair basename (string)\n\n" 
		<< "MODE :\te  = encrypt\nARGS :\ttext (string)\nKEY  :\tpublic key (path)\n\n" 
		<< "MODE :\tef = encrypt file\nARGS :\tfilename (path)\nKEY  :\tpublic key (path)\n\n"
		<< "MODE :\td  = decrypt\nARGS :\ttext (string)\nKEY  :\tprivate key (path)\n\n"  
		<< "MODE :\tdf = decrypt file\nARGS :\tfilename (path)\nKEY  :\tprivate key (path)\n\n"  
		<< endl;
}

int main(int argc, char** argv)
{
	string mode;
	if( argc > 3 )
	{
		mode = argv[1];

		// generate mode
		if(mode == "g")
		{
			Uint p = stoul(argv[2]);
			Uint q = stoul(argv[3]);
			string basename = argv[4];

			try
			{
				RSA<Uint> rsa(p,q);
				cout << rsa << endl;
				rsa.dump_keypair(basename);
			}
			catch(exception &e)
			{
				cerr << "EXC: " << e.what() << endl;
				exit(1);
			}
		}

		// encrypt mode
		else if (mode[0] == 'e')
		{
			string rsapub_f = argv[3];

			RSAenc<Uint> e(rsapub_f);
			cout << "Public key : " << e.n << " " << e.e << endl;

			if(mode == "e")
			{
				// handle string
			}
			else if (mode == "ef")
			{
				// handle file
			}
		}

		// decrypt mode
		else if (mode[0] == 'd')
		{
			string text = argv[2];
			string rsapri_f = argv[3];

			RSAdec<Uint> d(rsapri_f);
			cout << "Private key : " << d.n << " " << d.d << endl;

			if(mode == "d")
			{
				// handle string
			}
			else if (mode == "df")
			{
				// handle file
			}
		}
		else
		{
			cerr << "Unknown flag : " << mode << endl;
			usage(argv[0]);
			exit(1);
		}
	}
	else
	{
		usage(argv[0]);
		exit(2);
	}

	return 0;
}
