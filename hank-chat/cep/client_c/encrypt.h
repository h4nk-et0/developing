struct cep {

};

void encrypt_init(unsigned int *hash,unsigned int *seq_no,unsigned int *encrypt_seq)
{
	unsigned short int loop;

	for(loop = 0;loop < SEQ_LEN;loop++)
		encrypt_seq[loop] = hash[loop] ^ seq_no[loop];

}

void decrypt_init(unsigned int *hash,unsigned int *seq_no,unsigned int *decrypt_seq)
{
	unsigned short int loop;

	for(loop = 0;loop < SEQ_LEN;loop++)
		seq_no[loop] = hash[loop] ^ decrypt_seq[loop];

}

void rotate_seq_init(unsigned int *seq_no,unsigned int *rot_seq_no)
{

	rot_seq_no[0] = seq_no[4];
	rot_seq_no[1] = seq_no[3];
	rot_seq_no[2] = seq_no[2];
	rot_seq_no[3] = seq_no[1];
	rot_seq_no[4] = seq_no[0];

}

void encrypt()
{

}

void decrypt()
{

}
