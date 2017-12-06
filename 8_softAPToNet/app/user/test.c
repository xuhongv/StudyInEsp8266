/*
 Copyright (c) 2009 Dave Gamble

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 */

#include <stdio.h>
#include <stdlib.h>
#include "cJSON.h"

#include "c_types.h"
#include "cJSON.h"
#include "osapi.h"
#include "mem.h"
#include "ets_sys.h"
#include "osapi.h"

/* Parse text to JSON, then render back to text, and print! */
void ICACHE_FLASH_ATTR
doit(char *text) {

	os_printf("---------begging---------------\n");
	cJSON *root = cJSON_Parse(text);

	if (!root) {
		os_printf("Error before: [%s]\n", cJSON_GetErrorPtr());
		return;
	} else {

		int width, height;

		cJSON *width_json = cJSON_GetObjectItem(root, "width");

		if (width_json) {
			width = width_json->valueint;
			os_printf("width:%d\n", width);
		} else {
			os_printf(" width null! \n");
		}


		cJSON *height_json = cJSON_GetObjectItem(root, "height");
		if (height_json) {
			height = height_json->valueint;
			os_printf("height:%d\n", height);
			os_printf(" height ok! \n");
		} else {
			os_printf(" height null! \n");
		}

		cJSON_Delete(root);

	}
}

#if 0
/* Read a file, parse, render back, etc. */
void ICACHE_FLASH_ATTR
dofile(char *filename)
{
	FILE *f;long len;char *data;

	f=fopen(filename,"rb");fseek(f,0,SEEK_END);len=ftell(f);fseek(f,0,SEEK_SET);
	data=(char*)malloc(len+1);fread(data,1,len,f);data[len]='\0';fclose(f);
	doit(data);
	os_free(data);
}
#endif

/* Used by some code below as an example datatype. */
struct record {
	const char *precision;
	double lat, lon;
	const char *address, *city, *state, *zip, *country;
};

/* Create a bunch of objects as demonstration. */
void ICACHE_FLASH_ATTR
create_objects() {
	cJSON *root, *fmt, *img, *thm, *fld;
	char *out;
	int i; /* declare a few. */
	/* Our "days of the week" array: */
	const char *strings[7] = { "Sunday", "Monday", "Tuesday", "Wednesday",
			"Thursday", "Friday", "Saturday" };
	/* Our matrix: */
	int numbers[3][3] = { { 0, -1, 0 }, { 1, 0, 0 }, { 0, 0, 1 } };
	/* Our "gallery" item: */
	int ids[4] = { 116, 943, 234, 38793 };
#if 0
	/* Our array of "records": */
	struct record fields[2]= {
		{	"zip",37.7668,-1.223959e+2,"","SAN FRANCISCO","CA","94107","US"},
		{	"zip",37.371991,-1.22026e+2,"","SUNNYVALE","CA","94085","US"}};
#endif
	//volatile double zero = 0.0;
	volatile int zero = 0;

	/* Here we construct some JSON standards, from the JSON site. */

	/* Our "Video" datatype: */
	root = cJSON_CreateObject();
	cJSON_AddItemToObject(root, "name",
			cJSON_CreateString("Jack (\"Bee\") Nimble"));
	cJSON_AddItemToObject(root, "format", fmt = cJSON_CreateObject());
	cJSON_AddStringToObject(fmt, "type", "rect");
	cJSON_AddNumberToObject(fmt, "width", 1920);
	cJSON_AddNumberToObject(fmt, "height", 1080);
	cJSON_AddFalseToObject(fmt, "interlace");
	cJSON_AddNumberToObject(fmt, "frame rate", 24);

	out = cJSON_Print(root);
	cJSON_Delete(root);
	os_printf("%s\n", out);
	os_free(out); /* Print to text, Delete the cJSON, print it, release the string. */

	/* Our "days of the week" array: */
	root = cJSON_CreateStringArray(strings, 7);

	out = cJSON_Print(root);
	cJSON_Delete(root);
	os_printf("%s\n", out);
	os_free(out);

	/* Our matrix: */
	root = cJSON_CreateArray();
	for (i = 0; i < 3; i++)
		cJSON_AddItemToArray(root, cJSON_CreateIntArray(numbers[i], 3));

	/*	cJSON_ReplaceItemInArray(root,1,cJSON_CreateString("Replacement")); */

	out = cJSON_Print(root);
	cJSON_Delete(root);
	os_printf("%s\n", out);
	os_free(out);

	/* Our "gallery" item: */
	root = cJSON_CreateObject();
	cJSON_AddItemToObject(root, "Image", img = cJSON_CreateObject());
	cJSON_AddNumberToObject(img, "Width", 800);
	cJSON_AddNumberToObject(img, "Height", 600);
	cJSON_AddStringToObject(img, "Title", "View from 15th Floor");
	cJSON_AddItemToObject(img, "Thumbnail", thm = cJSON_CreateObject());
	cJSON_AddStringToObject(thm, "Url",
			"http:/*www.example.com/image/481989943");
	cJSON_AddNumberToObject(thm, "Height", 125);
	cJSON_AddStringToObject(thm, "Width", "100");
	cJSON_AddItemToObject(img, "IDs", cJSON_CreateIntArray(ids, 4));

	out = cJSON_Print(root);
	cJSON_Delete(root);
	os_printf("%s\n", out);
	os_free(out);

#if 0
	/* Our array of "records": */

	root=cJSON_CreateArray();
	for (i=0;i<2;i++)
	{
		cJSON_AddItemToArray(root,fld=cJSON_CreateObject());
		cJSON_AddStringToObject(fld, "precision", fields[i].precision);
		cJSON_AddNumberToObject(fld, "Latitude", fields[i].lat);
		cJSON_AddNumberToObject(fld, "Longitude", fields[i].lon);
		cJSON_AddStringToObject(fld, "Address", fields[i].address);
		cJSON_AddStringToObject(fld, "City", fields[i].city);
		cJSON_AddStringToObject(fld, "State", fields[i].state);
		cJSON_AddStringToObject(fld, "Zip", fields[i].zip);
		cJSON_AddStringToObject(fld, "Country", fields[i].country);
	}

	/*	cJSON_ReplaceItemInObject(cJSON_GetArrayItem(root,1),"City",cJSON_CreateIntArray(ids,4)); */

	out=cJSON_Print(root); cJSON_Delete(root); os_printf("%s\n",out);
	os_free(out);
#endif

	root = cJSON_CreateObject();
	//cJSON_AddNumberToObject(root,"number", 1.0/zero);
	cJSON_AddNumberToObject(root, "number", zero);
	out = cJSON_Print(root);
	cJSON_Delete(root);
	os_printf("%s\n", out);
	os_free(out);
}

int ICACHE_FLASH_ATTR
cJSON_TEST(void) {
	/* a bunch of json: */
	char text1[] =
			"\n { \"type\":\"rect\",\"width\":1920,\"height\":1080,\"interlace\":false,\"rate\":24}";

	char text2[] =
			"[\"Sunday\", \"Monday\", \"Tuesday\", \"Wednesday\", \"Thursday\", \"Friday\", \"Saturday\"]";
	char text3[] = "[\n    [0, -1, 0],\n    [1, 0, 0],\n    [0, 0, 1]\n	]\n";
	char text4[] =
			"{\n		\"Image\": {\n			\"Width\":  800,\n			\"Height\": 600,\n			\"Title\":  \"View from 15th Floor\",\n			\"Thumbnail\": {\n				\"Url\":    \"http:/*www.example.com/image/481989943\",\n				\"Height\": 125,\n				\"Width\":  \"100\"\n			},\n			\"IDs\": [116, 943, 234, 38793]\n		}\n	}";
	char text5[] =
			"[\n	 {\n	 \"precision\": \"zip\",\n	 \"Latitude\":  37.7668,\n	 \"Longitude\": -122.3959,\n	 \"Address\":   \"\",\n	 \"City\":      \"SAN FRANCISCO\",\n	 \"State\":     \"CA\",\n	 \"Zip\":       \"94107\",\n	 \"Country\":   \"US\"\n	 },\n	 {\n	 \"precision\": \"zip\",\n	 \"Latitude\":  37.371991,\n	 \"Longitude\": -122.026020,\n	 \"Address\":   \"\",\n	 \"City\":      \"SUNNYVALE\",\n	 \"State\":     \"CA\",\n	 \"Zip\":       \"94085\",\n	 \"Country\":   \"US\"\n	 }\n	 ]";

	/* Process each json textblock by parsing, then rebuilding: */
	doit(text1);
	//doit(text2);
	//doit(text3);
	//doit(text4);
	//doit(text5);

	/* Parse standard testfiles: */
	/*	dofile("../../tests/test1"); */
	/*	dofile("../../tests/test2"); */
	/*	dofile("../../tests/test3"); */
	/*	dofile("../../tests/test4"); */
	/*	dofile("../../tests/test5"); */
	/*	dofile("../../tests/test6"); */

	/* Now some samplecode for building objects concisely: */
	//create_objects();
	return 0;
}
