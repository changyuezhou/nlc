#ifndef LTS_COMM_STRUCT_H
#define LTS_COMM_STRUCT_H

typedef struct _proto_info_c
{
	//
	char *type_name;
	char *type_folder;
	char *key_name;
}proto_info_c;

typedef struct _other_c
{
	char *encoder;
	int max_line_len;
}other_c;

typedef struct _config_info_c
{
	char *name;
	char *value;
}config_info_c;


#endif
