#define DIGEST_LEN	80	

void sha_1(char *passwd, unsigned int *hash)
{
	unsigned short int	loop,
				in_loop;
	unsigned int	inter_hash[HASH_LEN],
			inter_const[3],
			msg_digest[DIGEST_LEN],
			size = 0;

	for(loop = strlen(passwd);loop < PASS_LEN;loop++)
		passwd[loop]='\0';

	for(loop = 0;loop < DIGEST_LEN;loop++)
		msg_digest[loop] = 0;

	in_loop = 0;
	loop = 0;
	while(1)
	{
		msg_digest[in_loop] = (msg_digest[in_loop] << 8) + (int)passwd[loop];

		loop++;
		if(loop % 4 == 0)
			in_loop++;
		if(loop == strlen(passwd))
		{
			int temp = 0;

			if(loop % 4 == 0)
				in_loop++;
			temp = loop;
			loop = 0;
			msg_digest[in_loop] = (msg_digest[in_loop] << 4) + 8;
			loop = loop + 4;

			while((temp * 8 + loop) % 32 != 0)
			{
				msg_digest[in_loop] = (msg_digest[in_loop] << 1) + 0;
				loop++;
			}
			break;
		}
	}

	for(loop = in_loop + 1;loop < 13;loop++)
		msg_digest[loop] = 0;

	size = 8 * strlen(passwd);
	msg_digest[14] = (0xffff0000) & (size);
	msg_digest[15] = (0x0ffff) & (size);

	for(loop = 16;loop < 80;loop++)
	{
		msg_digest[loop] = ((msg_digest[loop-3]) ^ (msg_digest[loop - 8]) ^ (msg_digest[loop - 14]) ^ (msg_digest[loop - 16]));
		msg_digest[loop] = ((msg_digest[loop] << 1) | (msg_digest[loop] >> 31));
	}

	hash[0] = 0x67452301;
	hash[1] = 0xefcdab89;
	hash[2] = 0x98badcfe;
	hash[3] = 0x10325476;
	hash[4] = 0xc3d2e1f0;

	inter_hash[0] = hash[0];
	inter_hash[1] = hash[1];
	inter_hash[2] = hash[2];
	inter_hash[3] = hash[3];
	inter_hash[4] = hash[4];

	for(loop = 0;loop < 80;loop++)
	{
		if(loop <= 19)
		{
			inter_const[0] = (inter_hash[1] & inter_hash[2]) | ((~inter_hash[1]) & inter_hash[3]);
			inter_const[1] = 0x5a827999;
		}

		else if(loop <= 39)
		{
			inter_const[0] = inter_hash[1] ^ inter_hash[2] ^ inter_hash[3];
			inter_const[1] = 0x6ed9eba1;
		}

		else if(loop <= 59)
		{
			inter_const[0] = (inter_hash[1] & inter_hash[2]) | (inter_hash[1] & inter_hash[3]) | (inter_hash[2] & inter_hash[3]);
			inter_const[1] = 0x8f1bbcdc;
		}

		else if(loop <= 79)
		{
			inter_const[0] = inter_hash[1] ^ inter_hash[2] ^ inter_hash[3];
			inter_const[1] = 0xca62c1d6;
		}

		inter_const[2] = (((inter_hash[0] << 5) | (inter_hash[0] >> 27)) + inter_const[0] + inter_hash[4] + inter_const[1] + msg_digest[loop]);
		inter_hash[4] = inter_hash[3];
		inter_hash[3] = inter_hash[2];
		inter_hash[2] = ((inter_hash[1] << 30) | (inter_hash[1] >> 2));
		inter_hash[1] = inter_hash[0];
		inter_hash[0] = inter_const[2];
	}

	hash[0] += inter_hash[0];
	hash[1] += inter_hash[1];
	hash[2] += inter_hash[2];
	hash[3] += inter_hash[3];
	hash[4] += inter_hash[4];

}
