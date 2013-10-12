#pragma once
// strcmp
#include <string.h>
#include <cstdlib>


#ifdef sqlite3
#include "sqlite3.h"
#endif

#include "netbase.hpp"
#include "util.hpp"
#include "import.hpp"
#include "relations.hpp"// for wordnet
#include "reflection.hpp"


// 64 BIT : %x -> %016llX
char* nodes_file = "nodes.txt";
char* statements_file = "statements.txt";
char* images_file = "images.txt";

void norm(char* title) {
	int len = strlen(title);
	for (int i = len; i >= 0; --i) {
		if (title[i] == ' ' || title[i] == '_' || title[i] == '-') {
			strcpy(&title[i], &title[i + 1]); //,len-i);
		}
	}
}
map<long, string> nodeNameImages;
map<long, string> nodeNameImages2; // chopped
//map<long,string> nodeNameImages3;// chopped image name

void importImages() {// 18 MILLION!   // 18496249

	p("image import starting ...");
	FILE *infile;
	char line[100];
	char* lastTitle = 0;
	int linecount = 0;
	Node* wiki_image = getAbstract("wiki_image");
	addStatement(wiki_image, is_a, getThe("image"));

	/* Open the file.  If NULL is returned there was an error */
	const char* file = (import_path + images_file).data();
	printf("Opening File %s\n", file);
	if ((infile = fopen(file, "r")) == NULL) {
		perror("Error opening file");
		perror(file);
		exit(1);
	}
	char tokens[1000];
	char image[1000];
	char *title = (char *) malloc(1000);
	int good = 0;
	int bad = 0;
	while (fgets(line, sizeof (line), infile) != NULL) {
		if (++linecount % 10000 == 0) {
			pi(linecount);
		};
		sscanf(line, "%s %*s %s", title, image);
		if (eq(lastTitle, title))
			continue;
		if (eq(title, "Alabama")) {
			printf("A! %s\n", title);
			//			lastTitle=0;
		}

		lastTitle = clone(title); // only the first
		if (!hasWord(title))
			norm(title); //blue -_fin ==> bluefin
		if (!hasWord(title)&&!hasWord(downcase(title)))
			continue; // currently only import matching words.

		//            if(++bad%1000==0){ps("bad image (without matching word) #");pi(bad);}
		//		if (getImage(title) != "")
		//			continue; //already has one ; only one so far!
		Node* subject = getAbstract(title);
		Node* object = getAbstract(image); // getThe(image);;
		//		if(endswith(image,".ogg")||endswith(image,".mid")||endswith(image,".mp3"))
		addStatement(subject, wiki_image, object, false);
		printf("%s\n", title);
		if (!eq(title, downcase(title)))
			addStatement(getAbstract(downcase(title)), wiki_image, object, false);
		if (++good % 10000 == 0) {
			ps("GOOD images:");
			pi(good);
		}
	}
	fclose(infile);

	good = 0;
	Node* object = getAbstract(image); // getThe(image);;
	/*
	// again, this time with word fragments
	 * MEMORY LEAK!? where??
	infile = fopen((import_path+ images_file).data(), "r");
	  while (fgets(line, sizeof (line), infile) != NULL) {
		if (++linecount % 1000 == 0)pi(linecount);
		sscanf(line, "%s %*s %s", title, image);
//        #blue fin ==> blue and fin
		char* word=title;
		for (int i = 0; i < strlen(title); i++) {
			if(title[i]==' '||title[i]=='_'||title[i]=='-'){//split
				title[i]=0;
//                nodeNameImages2[hash(title)]=image;
				if(!hasWord(word))continue;
				if(getImage(word)!="")continue;// only one so far!
				addStatement(getAbstract(word), wiki_image, object,false);
				if(++good%1000==0){ps("!!good");pi(good);}

				word=&title[i+1];
			}
		}
	}
	 */
	//    for(map<long,string>::const_iterator iter = nodeNameImages2.begin(); iter != nodeNameImages2.end(); ++iter){
	//        long key=iter->first;
	//        string value=iter->second;
	//        if(nodeNameImages.find(key)==nodeNameImages.end())// does not exist?
	//            nodeNameImages[key]=value;
	//    }
	//    for(map<long,Node*>::const_iterator iter = abstracts->begin(); iter != abstracts->end(); ++iter){
	//        long key=iter->first;
	//        Node* node=iter->second;
	//        string image=nodeNameImages[key];
	////        if(!image)key=hash2(node->name)
	//        Node* object = getAbstract(image.data());// getThe(image);;
	//        addStatement(node, predicate, object,false);
	//    }
	fclose(infile);
	p("done image import");
}

void importNodes() {
	p("node import starting ...");
	FILE *infile;
	// char fname[40]="/Users/me/data/base/netbase.sql";
	//  ('2282','Anacreon','N'),

	// char* fname="/Users/me/data/base/netbase/nodes.test";
	//2026896532	103	dry_out	Verb	\N	\N	11	103

	char line[100];
	int linecount = 0;
	/* Open the file.  If NULL is returned there was an error */
	printf("Opening File %s\n", (data_path + nodes_file).data());
	if ((infile = fopen((data_path + nodes_file).data(), "r")) == NULL) {
		perror("Error opening file");
		exit(1);
	}
	while (fgets(line, sizeof (line), infile) != NULL) {
		char tokens[1000];
		/* Get each line from the infile */

		if (++linecount % 1000 == 0)
			pi(linecount);
		strcpy(tokens, line);
		int x = 0; // replace ' ' with '_'
		while (tokens[x]) {
			if (tokens[x] == ' ')
				tokens[x] = '_';
			x++;
		}
		// char name[1000];
		char* name = (char*) malloc(100);
		// char kind[20];
		char contextId_s[100];
		char deleted[1];
		char version[1];
		char wordKindName[100];
		int wordKind;
		int Id;
		int kind;
		int contextId = wordnet;
		int contextID;
		//2026896532	103	dry_out	Verb	\N	\N	11	103
		//	sscanf(tokens,"%d\t%s\t%s\t%s\t%*s\t%*s\t%*d\t%d",&Id,contextId_s,name,wordKindName,&kind);
		sscanf(tokens, "%d\t%s\t%s\t%*s\t%*s\t%*s\t%d\t%*d", &Id, contextId_s, name, &kind); // wordKind->kind !
		if (kind == 105)kind = _relation; //relation
		if (kind == 1)kind = _concept; //noun
		//        if(kind==1)continue;
		if (kind == 103)kind = noun; //noun
		if (kind == 10)kind = noun; //noun
		if (kind == 11)kind = verb; //verb
		if (kind == 12)kind = adjective; //adjective
		if (kind == 13)kind = adverb; //adverb
		//        contextId = atoi(contextId_s);
		//
		//        if (Id < 1000)contextId = wordnet; //wn
		//        if (contextId == 100 && Id > 1000)contextId = 100;
		Node* n;
		if (Id > 1000)
			n = add(name, kind, contextId);
		else
			n = add_force(contextId, Id, name, kind);
		if (n == 0) {
			p("out of memory");
			break;
		}
		wn_map[Id] = n->id;
		wn_map2[n->id] = Id;
	}
	fclose(infile); /* Close the file */
	p("node import ok");
}

void importStatements() {
	FILE *infile;

	char line[1000];
	int linecount = 0;

	/* Open the file.  If NULL is returned there was an error */
	printf("Opening File %s\n", (data_path + statements_file).data());
	if ((infile = fopen((data_path + statements_file).data(), "r")) == NULL) {
		perror("Error opening file");
		exit(1);
	}
	char tokens[1000];
	while (fgets(line, sizeof (line), infile) != NULL) {
		/* Get each line from the infile */
		if (++linecount % 1000 == 0)
			pi(linecount);
		// p(line);
		//	strcpy(  tokens,line);
		//		int x = 0;
		//    while (tokens[x++])
		//        if (tokens[x]==' ')
		//		    tokens[x]='_';
		int contextId;
		int subjectId;
		int predicateId;
		int objectId;
		int id; // ignore now!!
		sscanf(line, "%d\t%d\t%d\t%d", &id, &subjectId, &predicateId, &objectId); // no contextId? cause this db assumes GLOBAL id!
		subjectId = wn_map[subjectId]; //%1000000;//wn!!
		predicateId = predicateId; //%1000000;
		if (predicateId > 100)
			predicateId = wn_map[predicateId]; //%1000000;
		objectId = wn_map[objectId]; //%1000000;
		// printf("%d\t%d\t%d\n",subjectId,predicateId,objectId);
		if (subjectId < 1000 || objectId < 1000 || predicateId == 50 || predicateId == 0 || subjectId == 1043 || subjectId == 1044)continue;
		Statement* s = addStatement4(wordnet, subjectId, predicateId, objectId);
	}
	fclose(infile); /* Close the file */
	p("statements import ok");
}

#ifdef sqlite3
int maxBytes = 1000000;

void importSqlite(char* filename) {
	sqlite3* db;
	sqlite3_stmt *statement;
	const char* unused = (char*) malloc(1000);
	int status = sqlite3_open(filename, &db);
	pi(status);
	status = sqlite3_prepare(db, "select * from nodes;", maxBytes, &statement, &unused);
	pi(status);

	// http://www.sqlite.org/c3ref/step.html
	status = sqlite3_step(statement);
	pi(status);
	if (status == SQLITE_DONE) {
		p("No results");
	}
	if (status == SQLITE_ROW) {
		// http://www.sqlite.org/c3ref/column_blob.html
		int id = sqlite3_column_int(statement, 0);
		const unsigned char *text = sqlite3_column_text(statement, 1);
		printf("text %s\n", text);
	}
	sqlite3_finalize(statement);
	sqlite3_close(db);
}
#endif

string deCamel(string s) {
	static string space = " ";
	for (int i = s.length(); i > 1; i--) {
		char c = s[i];
		if (c > 65 && c < 91)s.replace(i, 1, space + (char) tolower(c));
		if (c == '(')s[i - 1] = 0; //cut _( )
		if (c == ',')s[i] = 0; //cut ,_
		if (c == ')')s[i] = 0;
	}
	//    s[0]=tolower(s[0]);
	s = replace_all(s, "_", " ");
	s = replace_all(s, "  ", " ");
	return s;
};

// TODO :!test MEMLEAK!

const char* parseWikiTitle(char* item, int id = 0, int context = current_context) {
	//    string s="wordnet_raw_material_114596700";
	static string s;
	s = string(item); //"Uncanny_Tales_(Canadian_pulp_magazine)";
	//    South_Taft,_California
	// facts/hasPopulationDensity/ArticleExtractor.txt:205483101	Sapulpa,_Oklahoma	397.1#/km^2	0.9561877303748038


	//    string s="wordnet_yagoActorGeo_1";
	//    wordnet_yagoActorGeo_1
	//    if(s.find("wordnet"))contextId=wordnet;
	//if(s.find("wikicategory")) kind=list / category
	s = replace_all(s, "wikicategory_", "");
	s = replace_all(s, "wordnet_(.*)_(\\d*)", "$1"); // todo
	s = replace_all(s, "yago_", "");
	s = replace_all(s, "wikicategory", "");
	s = replace_all(s, "wordnet", "");
	s = replace_all(s, "yago", "");
	int last = s.rfind("_");
	int type = s.find("(");
	string clazz = deCamel(s.substr(type + 1, -1));
	string word = s;
	string Id = s.substr(last + 1, -1);
	//    int
	id = atoi(Id.c_str());
	if (id > 0) {
		word = s.substr(0, last);
	}
	//    word=deCamel(word);
	//    item=word.c_str();
	//    ~word;
	return word.c_str(); // TODO!! MEMLEAK!!
}

void extractUnit() {
	string s = "397.1#/km^2";
}

char* charToCharPointer(char c) {
	char separator[2] = {c, 0};
	char* separatorx = separator;
	return separatorx;
}

char guessSeparator(char* line) {
	const char* separators = ",\t;|";
	char the_separator = '\t';
	int max = 0;
	int size = strlen(separators);
	for (int i = 0; i < size; i++) {
		int nr = splitStringC(line, 0, (char) separators[i]);
		if (nr > max) {
			the_separator = separators[i];
			max = nr;
		}
	}
	return the_separator;
}

int getNameRow(char** tokens, int nameRowNr = -1, const char* nameRow = 0) {
	int row = 0;
	while (true) {
		char* token = tokens[row++];
		if (!token)break;
		if (nameRowNr < 0) {
			if (nameRow == 0) {
				if (eq("name", token))nameRowNr = row;
				if (eq("Name", token))nameRowNr = row;
				if (eq("title", token))nameRowNr = row;
				if (eq("Title", token))nameRowNr = row;
			} else if (eq(nameRow, token))nameRowNr = row;
		}
	}
	return nameRowNr;
}

// BROKEN!!!??!!

int getFields(char* line, vector<char*>& fields, char separator, int nameRowNr, const char* nameRow) {
	char * token;
	char* separators = ",;\t|";
	if (separator)
		separators = charToCharPointer(separator);
	//	line=modifyConstChar(line);
	token = strtok(line, separators);
	int row = 0;
	while (token != NULL) {
		fields.push_back(token);
		token = strtok(NULL, separators); // BROKEN !!??!!??!!??
		row++;
	}
	for (int i = 0; i < row; i++) {
		token = fields.at(i);
		if (nameRowNr < 0) {
			if (nameRow == 0) {
				if (eq("name", token))nameRowNr = row;
				if (eq("Name", token))nameRowNr = row;
				if (eq("title", token))nameRowNr = row;
				if (eq("Title", token))nameRowNr = row;
			} else if (eq(nameRow, token))nameRowNr = row;
		}
	}
	return nameRowNr;
}

void fixNewline(char* line) {
	int len = strlen(line);
	if (len == 0)return;
	if (line[len - 1] == '\n')
		line[--len] = 0;
	if (line[len - 1] == '\r')
		line[--len] = 0;
	if (line[len - 1] == '\t')
		line[--len] = 0;
}

char* extractTagName(char* line) {
	return match(line, "<([^>]+)>");
}

char* extractTagValue(char* line) {
	return match(line, ">([^<]+)<");
}

bool isNameField(char* field, char* nameField) {
	if (nameField && !eq(field, nameField))return false;
	if (eq(field, nameField))return true;
	if (eq(field, "name"))return true;
	if (eq(field, "title"))return true;
	if (eq(field, "label"))return true;
	return false;
}

Node* namify(Node* node, char* name) {
	Context* context = currentContext();
	node->name = &context->nodeNames[context->currentNameSlot];
	strcpy(node->name, name); // can be freed now!
	int len = strlen(name);
	context->nodeNames[context->currentNameSlot + len] = 0;
	addStatement(getAbstract(name), Instance, node, DONT_CHECK_DUPLICATES);
	// replaceNode(subject,object);
	return node;
}

void addAttributes(Node* subject, char* line) {
	line = (char*) replace_all(line, "\"", "'").c_str();
	do {
		char* attribute = match(line, " ([^=]+)='[^']'");
		char* value = match(line, " [^=]+='([^'])'");
		if (!attribute)break;
		Node* predicate = getThe(attribute);
		Node* object = getThe(value);
		addStatement(subject, predicate, object);
	} while (attribute != 0);
}

bool hasAttribute(char* line) {
	return match(line, " ([^=]+)=");
}

//Context* context,
//Node* type,
// importXml("/Users/me/data/base/geo/geolocations/Orte_und_GeopIps_mit_PLZ.xml","city","ort");

void importXml(const char* facts_file, char* nameField, const char* ignoredFields, const char* includedFields) {
	p("import csv start");
	bool dissect = false;
	char line[1000];
	char* line0 = (char*) malloc(sizeof (char*) *100);
	char* field = (char*) malloc(sizeof (char*) *100);
	char* value = (char*) malloc(sizeof (char*) *10000000);// uiuiui!

	Node* root = 0;
	Node* parent = 0; // keep track of 1 layer
	Node* subject = 0;
	Node* predicate = 0;
	Node* object = 0;
	vector<Node*> predicates = *new vector<Node*>();
	map<char*, char*> fields;
	queue<Node*> parents; //constructed!!

	//	char* objectName = (char*) malloc(100);
	//	int depth = 0;
	//	vector<char*> ignoreFields = splitString(ignoredFields, ",");
	//	vector<char*>& includeFields = splitString(includedFields, ",");
	int linecount = 0;
	FILE *infile;
	printf("Opening XML File %s\n", (facts_file));
	if ((infile = fopen((facts_file), "r")) == NULL)facts_file = (import_path + facts_file).c_str();
	if ((infile = fopen((facts_file), "r")) == NULL) {
		perror("Error opening file");
		exit(1);
	}
	Node* UNKNOWN_OR_EMPTY = getThe("<unknown/>");
	//	map<Node*,Node*> fields;
	while (fgets(line, sizeof (line), infile) != NULL) {
		if (!line)break;
		if (++linecount % 1000 == 0)printf("%d\n", linecount);
		fixNewline(line);
		line0 = line; // readable in Debugger!
		if (contains(line, "<?xml"))continue;
		if (!subject) {
			if (root)
				subject = add(extractTagName(line));
			else
				root = add(extractTagName(line));
			continue;
		}

		if (startsWith(line, "</")) {
			//			for(Node* predicate:fields){
			//				object = getThe(extractTagValue(line),null, dissect);
			//				Statement *s = addStatement(subject, predicate, object, false);
			//				showStatement(s);
			//			}
			//			if(!parents.empty())
			subject = 0; // parents.back(); // back????
			//			parents.pop();
			fields.clear();
			continue;
		}
		// <address> <city> <zip>12345</zip> <name>bukarest</name> </city> </address>
		if (match(line, "<[a-z]") && contains(line, "</")) {
			field = extractTagName(line);
			value = extractTagValue(line);
			if (ignoredFields && contains(ignoredFields, field))
				continue;
			if (isNameField(field, nameField)) {
				//				parent=parents.front();
				// rewrite address.member=city
				//				deleteStatement(findStatement(parent, Member, subject, 0, 0, 0, 0));
				// address.CITY=bukarest
				predicate = getThe(subject->name); // CITY
				Node* object = namify(subject, value); // city.name=bukarest
				addStatement(parent, predicate, object, DONT_CHECK_DUPLICATES); // address.CITY=bukarest
				subject = object; // bukarest
				//				show(subject,true);
				continue;
			}

			if (!value) {//<address> <city> ...
				//				parents.push(subject);
				parent = subject;

				//			if(eq(field,"zip")){
				//				value=value;
				//				showStatement(s);
				//			}

				if (!contains(line, "><")) {// else empty!
					object = add(field);
					addStatement(subject, Member, object, DONT_CHECK_DUPLICATES);
					subject = object;
				} else {
					object = getThe(field);
					//					addStatement(subject, Unknown,object,DONT_CHECK_DUPLICATES);//EMPTY
					addStatement(subject, object, Unknown, DONT_CHECK_DUPLICATES); //EMPTY
					//					addStatement(subject, object,UNKNOWN_OR_EMPTY,DONT_CHECK_DUPLICATES);//EMPTY
				}
				addAttributes(subject, line);
				continue;
			}
			if (hasAttribute(line)) {
				predicate = add(field); // <zip color='green'>12345</zip>
				addAttributes(predicate, line);
			} else {
				predicate = getThe(field, NO_TYPE, DONT_DISSECT);
			}
			object = getThe(value, NO_TYPE, DONT_DISSECT);
			Statement* s = addStatement(subject, predicate, object, DONT_CHECK_DUPLICATES);

			//			fields.insert(predicate,object);//
			continue;
		}
		if (startsWith(line, "<") && !contains(line, "</")) {
			parent = subject;
			//			parents.push(subject); // <address> <city> ...
			field = extractTagName(line);
			subject = add(field); // can be replaced by name!
			addStatement(parent, Member, subject, DONT_CHECK_DUPLICATES); // address.city
			addAttributes(subject, line);
			continue;
		}
	}
	fclose(infile); /* Close the file */
	p("import xml ok ... items imported:");
	pi(linecount);
}

void importCsv(const char* facts_file, Node* type, char separator, const char* ignoredFields, const char* includedFields, int nameRowNr, const char* nameRow) {
	p("import csv start");
	char line[1000];
	char** values = (char**) malloc(sizeof (char*) *100);
	memset(values, 0, 100);
	//    vector<char*> values;

	Node* subject = 0;
	Node* predicate = 0;
	Node* object = 0;
	vector<Node*> predicates = *new vector<Node*>();
	vector<char*> ignoreFields = splitString(ignoredFields, ",");
	vector<char*>& includeFields = splitString(includedFields, ",");
	//	vector<char*>& fields = *new vector<char*>();
	int linecount = 0;
	FILE *infile;
	printf("Opening File %s\n", (facts_file));
	if ((infile = fopen((facts_file), "r")) == NULL)facts_file = (import_path + facts_file).c_str();
	if ((infile = fopen((facts_file), "r")) == NULL) {
		perror("Error opening file");
		exit(1);
	}
	char* objectName = (char*) malloc(100);
	int fieldCount = 0;
	char* columnTitles;
	while (fgets(line, sizeof (line), infile) != NULL) {
		if (!line)break;
		fixNewline(line);
		if (linecount == 0) {
			columnTitles = line;
			if (!separator)
				separator = guessSeparator(line);
			fieldCount = splitStringC(line, values, separator);
			nameRowNr = getNameRow(values, nameRowNr, nameRow);
			//						getFields(line, fields, separator, nameRowNr, nameRow);// vector ok, only once!

			//			check(fields.size() == fieldCount)
			//				if (fields.size() == 0)
			for (int i = 0; i < fieldCount; i++) {
				char* field = values[i];
				//				char* field = fields.at(i);
				predicates.push_back(getThe(field));
			}
			++linecount;
			continue;
		}
		if (++linecount % 100 == 0)printf("%d\n", linecount);

		//        values.erase(values.begin(),values.end());
		//        ps(line);

		int size = splitStringC(line, values, separator);
		if (fieldCount != size) {
			printf("Warning: fieldCount!=columns in line %d   (%d!=%d)\n%s\n", linecount - 1, fieldCount, size, line);
			//            ps(columnTitles); // only 1st word:
			//            ps(line);// split! :{
			continue;
		}

		bool dissect = type && !eq(type->name, "city"); // city special: too many!
		// todo more generally : don't dissect special names ...

		subject = getThe(values[nameRowNr], null, dissect);
		if (type && subject->kind != type->id) {
			//			p("Found one with different type");
			Node* candidate = subject;
			subject = getThe(values[nameRowNr], type, dissect); // todo : match more or less strictly? EG Hasloh
			addStatement4(type->context, candidate->id, Synonym->id, subject->id, true);
			//			addStatement4(type->context,candidate->id,Unknown->id,subject->id,false);
		}
		//		if(eq(subject->name,"Hasloh"))
		//			linecount++;
		for (int i = 0; i < size; i++) {
			if (i == nameRowNr)continue;
			predicate = predicates[i];
			if (predicate == null)continue;
			if (contains(ignoreFields, predicate->name))continue;
			if (includedFields != null && !contains(includeFields, predicate->name))continue;
			char* vali = values[i];
			if (!vali || strlen(vali) == 0)continue; //HOW *vali<100??
			object = getThe(vali);
			Statement *s = addStatement(subject, predicate, object, false);
			//            showStatement(s);
		}

		if (!subject || !predicate || !object || subject->id > maxNodes || object->id > maxNodes) {
			printf("Quitting import : id > maxNodes\n");
			break;
		}
	}
	fclose(infile); /* Close the file */
	p("import csv ok ... lines imported:");
	pi(linecount);
}

void importList(const char* facts_file, const char* type) {
	p("import list start");
	char line[1000];
	Node* subject = getClass(type);
	Node* object;
	int linecount = 0;
	FILE *infile;
	printf("Opening File %s\n", (facts_file));
	if ((infile = fopen((facts_file), "r")) == NULL) {
		perror("Error opening file");

		exit(1);
	}
	while (fgets(line, sizeof (line), infile) != NULL) {
		if (++linecount % 10000 == 0)printf("%d\n", linecount);
		char* objectName = (char*) malloc(100);
		object = getThe(line);
		addStatement(subject, Instance, object, false);
		if (!subject || !object || subject->id > maxNodes || object->id > maxNodes) {
			printf("Quitting import : id > maxNodes\n");
			break;
		}
	}
	fclose(infile); /* Close the file */
	p("import list ok");
}

char *fixRdfName(char *key) {
	if (key[0] == '<')key++;
	char* start = strstr(key, ".");
	if (!start)start = key;
}

char *cut_wordnet_id(char *key) {
	for (int i = strlen(key); i > 1; --i) {
		if (key[i] == '_') {
			char* id = key + i + 1 + 1;
			key[i] = 0;
			return id; // <wordnet_album_106591815> ->  06591815 DROP LEADING 1 !!!
		}
	}
	return 0;
}

bool hasCamel(char* key) {
	if (contains(key, "_"))return false;
	char last = key[0];
	for (int i = 0; i < strlen(key); i++) {
		char c = key[i];
		if (c > 64 && c < 91)
			if (last > 96 && last < 123) {
				pi(i);
				return true;
			}
		last = c;
	}
	return false;
}

char* removeHead(char *key, char *bad) {
	char* start = strstr(key, bad);
	if (start)key = key + strlen(bad);
	return key;
}

const char *fixYagoName(char *key) {
	if (key[0] == '<')key++;
	int len = strlen(key);
	if (key[len - 1] == '>')key[len - 1] = 0;
	key = removeHead(key, "wikicategory_");
	key = removeHead(key, "wordnetDomain_");
	char* start = strstr(key, "wordnet_");
	if (start) {
		char* id = cut_wordnet_id(start);
		key = removeHead(key, "wordnet_");
	}
	//	if(hasCamel(key)) // McBain
	//		return deCamel(key).data();
	return key;
}

Node* getYagoConcept(char* key) {

	// Normalized instead of using similar
	if (eq(key, "rdf:type"))return Type;
	if (eq(key, "rdfs:subClassOf"))return SuperClass;
	if (eq(key, "rdfs:label"))return Label;
	if (eq(key, "isPreferredMeaningOf"))return Label;
	if (eq(key, "skos:prefLabel"))return Label;
	if (eq(key, "rdfs:Property"))return Relation;
	if (eq(key, "rdf:Property"))return Relation;
	if (eq(key, "rdfs:subPropertyOf"))return SubClass;
	if (eq(key, "owl:SymmetricProperty"))return Relation; // symmetric!
	if (eq(key, "owl:TransitiveProperty"))return Relation; // transitive!
	if (eq(key, "rdfs:domain"))return Domain;
	if (eq(key, "rdfs:range"))return Range;

	if (eq(key, "rdfs:comment"))return Comment;
	if (eq(key, "rdf:Statement"))return get(_statement);
	if (eq(key, "rdfs:Class"))return Class;
	if (eq(key, "rdfs:class"))return Class;
	if (eq(key, "rdf:Resource"))return get(_node); // no info!
	if (eq(key, "rdfs:Resource"))return get(_node); // no info!
	if (eq(key, "rdfs:Literal"))return get(_node); // no info!
	if (eq(key, "xsd:string"))return get(_node); // no info!
	if (eq(key, "owl:Thing"))return get(_node); // no info!
	if (eq(key, "xsd:date"))return Date;
	if (eq(key, "xsd:decimal"))return Number;
	if (eq(key, "xsd:integer"))return Number;
	if (eq(key, "xsd:boolean"))return getAbstract("boolean"); // !
	if (eq(key, "xsd:gYear"))return getAbstract("year"); // !
	if (eq(key, "owl:disjointWith"))return getAbstract("disjoint with"); // symmetric!
	//	if(eq(key,"xsd:string"))return Text;// no info!
	if (eq(key, "xsd:nonNegativeInteger"))return getAbstract("natural number");
	if (eq(key, "owl:FunctionalProperty"))return Label;
	if (eq(key, "#label"))return Label;
	if (eq(key, "<label>"))return Label;
	if (eq(key, "<hasWordnetDomain>"))return Domain;
	//	if(eq(key,"owl:FunctionalProperty"))return Transitive;
	if (contains(key, "^^")) {
		char** all = splitStringC(key, '^');
		char* unit = all[2];
		key = all[0];
		key++; // ignore quotes "33"
		free(all);
		if(eq(unit,",)"))return 0;// LOL_(^^,) BUG!
		if (eq(unit, "xsd:integer"))unit = 0; //-> number
		if (eq(unit, "xsd:decimal"))unit = 0; //-> number
		if (eq(unit, "xsd:float"))unit = 0; //-> number
		if (eq(unit, "xsd:nonNegativeInteger"))unit = 0; //-> number
		else if (eq(unit, "<yago0to100>"))unit=0;
		if (!unit) return value(key, atoi(key), unit);

		if (eq(unit, "<m"))unit="Meter";
		else if (eq(unit, "<m>"))unit="Meter";
		else if (eq(unit, "<s>"))unit="Seconds";
		else if (eq(unit, "<g>"))unit="Gram";
		else if (eq(unit, "</km"))unit="Kilometer";
		else if (eq(unit, "xsd:date")); // parse! unit = 0; //-> number
		else if (eq(unit, "<degrees>")); // ignore
		else if (eq(unit, "<dollar>")); // ignore
		else if (eq(unit, "<euro>")); // ignore
		else if (eq(unit, "<yagoISBN>"))unit="ISBN"; // ignore
		else if (eq(unit, "<yagoTLD>"))
			unit="TLD"; // ???
		else if (eq(unit, "<yagoMonetaryValue>"))unit="dollar";
		else if (eq(unit, "<%>"))unit="%";// OK
		else if (eq(unit, "%"));// OK
		else printf("UNIT %s \n", unit);
//		, current_context, getYagoConcept(unit)->id
		return add(key);
		Node* unity=getYagoConcept(unit);
		return getThe(key,unity);
	}
	if (!startsWith(key, "<wiki") && contains(key, ":")) {
		printf(" unknown key %s\n", key);
		return 0;
	}
	if (contains(key, ".jpg") || contains(key, ".gif") || contains(key, ".svg") || startsWith(key, "#") || startsWith(key, "<#")) {
		printf(" bad key %s\n", key);
		return 0;
	}

	if(eq(key,"<Carding>"))
		return 0;// What the bug???
	return getThe(fixYagoName(key));

}

bool importYago(const char* facts_file) {
	p("import YAGO start");
	if (!contains(facts_file, "/"))
		facts_file = concat("/data/base/BIG/yago/", facts_file);
//		facts_file = concat("/Users/me/data/base/BIG/yago/", facts_file);
	Node* subject;
	Node* predicate;
	Node* object;
	int linecount = 0;
	FILE *infile;
	printf("Opening File %s\n", facts_file);
	if ((infile = fopen(facts_file, "r")) == NULL) {
		perror("Error opening file");
		return false;
	}
	char* line = (char*) malloc(1000);
	char* id = (char*) malloc(100);
	char* subjectName = (char*) malloc(100);
	char* predicateName = (char*) malloc(100);
	char* objectName = (char*) malloc(100);
	while (fgets(line, 1000, infile) != NULL) {
		fixNewline(line);
		if (linecount % 10000 == 0) {
			size_t currentSize = getCurrentRSS();
			size_t peakSize = getPeakRSS();
			size_t free = getFreeSystemMemory();
			//			printf("MEMORY: %L            Peak: %d FREE: %L \n", currentSize, peakSize, free);
			if (currentSize > 3*GB || currentSize*1.2>sizeOfSharedMemory|| currentContext()->nodeCount*1.2 > maxNodes) {
				printf("MEMORY safety boarder reached");
				return 0; //exit(0);// 4GB max
			}
		}
		if (++linecount % 10000 == 0)
			printf("%d\n", linecount);
		//			sscanf(line, "%s\t%s\t%s\t%s", &id, subjectName,predicateName, objectName /*, &certainty*/);
		if (line[0] == '\t')line[0] = ' '; // don't line++ , else nothing left!!!
		//		printf("%s\n", line);
		int ok = sscanf(line, "<%s>\t<%s>\t%s\t<%s>", id, subjectName, predicateName, objectName /*, &certainty*/);
		if (!ok)ok = sscanf(line, "<%s>\t%s\t<%s>", subjectName, predicateName, objectName /*, &certainty*/);
		if (!ok)ok = sscanf(line, "%s\t%s\t%s", subjectName, predicateName, objectName /*, &certainty*/);
		if (!ok)ok = sscanf(line, "%s\t%s\t%s\t%s", id, subjectName, predicateName, objectName);
		if (ok < 3) {
			//			printf("%s\n", line);
			char** all = splitStringC(line, '\t');
			if (all[2] == 0) {
				printf("ERROR %s\n", line);
				continue;
			}
			subject = getYagoConcept(all[1]); //
			predicate = getYagoConcept(all[2]);
			object = getYagoConcept(all[3]);
			free(all);
		} else {
			subject = getYagoConcept(subjectName); //
			predicate = getYagoConcept(predicateName);
			object = getYagoConcept(objectName);
		}
		if (subject == 0 || predicate == 0 || object == 0) {
			printf("ERROR %s\n", line);
			continue;
		}
		//dissectWord(abstract);
		Statement* s;
		//		if (contains(objectName, subjectName, true))
		//			s = addStatement(subject, Member, object, false); // todo: id
		//		else
		s = addStatement(subject, predicate, object, false); // todo: id
		if (!subject || !object || subject->id > maxNodes || object->id > maxNodes) {
			printf("Quitting import : id > maxNodes\n");
			break;
		}
		//		showStatement(s);
	}
	fclose(infile); /* Close the file */
	p("import facts ok");
	return true;
}

bool importFacts(const char* facts_file, const char* predicateName = "population") {
	p("import facts start");
	printf("WARNING: SETTING predicateName %s", predicateName);
	Node* subject;
	Node* predicate;
	Node* object;
	char line[1000];
	//    char* predicateName=(char*) malloc(100);
	predicate = getClass(predicateName);
	int linecount = 0;
	FILE *infile;
	printf("Opening File %s\n", facts_file);
	if ((infile = fopen(facts_file, "r")) == NULL) {
		perror("Error opening file");
		return false;
		//        exit(1);
	}
	char* objectName = (char*) malloc(100);
	char* subjectName = (char*) malloc(100);
	while (fgets(line, sizeof (line), infile) != NULL) {
		/* Get each line from the infile */
		if (++linecount % 10000 == 0)printf("%d\n", linecount);
		if (!eq(predicateName, "population"))
			sscanf(line, "%s\t%s", subjectName, objectName); // no contextId? cause this db assumes GLOBAL id!
		else
			sscanf(line, "%*d\t%s\t%s\t%*d", /*&id,*/ subjectName, objectName /*, &certainty*/); // no contextId? cause this db assumes GLOBAL id!
		// printf(line);
		//	 printf("%d\t%d\t%d\n",subjectId,predicateId,objectId);
		//		if (contains(subjectName, "Begriffskl") || contains(subjectName, "Abkürzung") || contains(subjectName, ":") || contains(subjectName, "#"))
		//			continue;
		if (contains(objectName, "jpg") || contains(objectName, "gif") || contains(objectName, "svg") || contains(objectName, "#") || contains(objectName, ":"))
			continue;

		subject = getAbstract(subjectName); //
		object = getAbstract(objectName);
		//dissectWord(abstract);
		Statement* s;
		if (contains(objectName, subjectName, true))
			s = addStatement(subject, Member, object, false); // todo: id
		else
			s = addStatement(subject, predicate, object, false); // todo: id
		if (!subject || !object || subject->id > maxNodes || object->id > maxNodes) {
			printf("Quitting import : id > maxNodes\n");
			break;
		}
		showStatement(s);
	}
	fclose(infile); /* Close the file */
	p("import facts ok");
	return true;
}

void importNames() {
	importList((import_path + "FrauenVornamen.txt").data(), "female_firstname");
	importList((import_path + "MaennerVornamen.txt").data(), "male_firstname");
	addStatement(all(firstname), are, a(name));
	addStatement(all(male_firstname), a(gender), a(male));
	addStatement(all(male_firstname), Owner, a(male));
	addStatement(all(female_firstname), a(gender), a(female));
	addStatement(all(female_firstname), Owner, a(female));
}

void importWordnet() {
	importNodes(); // FIRST! Hardlinked ids overwrite everything!!
	importStatements();
}

void importGeoDB() {
	importCsv("cities1000.txt",\
			getThe("city"), '\t', "alternatenames,modificationdate,geonameid",\
		"latitude,longitude,population,elevation,countrycode", 2, "asciiname");
}

// IMPORTANT: needs manual collectAbstracts() afterwards (for speed reasons??)

void importAll() {
	//	importFacts()
	//	importCsv("adressen.txt");
	importNames();
	importWordnet();
	importGeoDB();
	//	if (getImage("alabama") != "" && getImage("Alabama") != "")
	//		p("image import done before ...");
	//	else
	//		importImages();
}

void importWikipedia() {
}

void importAllYago() {
	import("yago", "yagoStatistics.tsv");
	import("yago", "yagoSchema.tsv");
	import("yago", "yagoGeonamesClassIds.tsv");
	//import("yago","yagoGeonamesClasses.tsv");
	import("yago", "yagoGeonamesGlosses.tsv");
	import("yago", "yagoSimpleTaxonomy.tsv");
	//import("yago","yagoWordnetIds.tsv");// hasSynsetId USELESS!!!
	import("yago", "yagoGeonamesEntityIds.tsv");
	//import("yago","yagoWordnetDomains.tsv");
	//import("yago","yagoMultilingualClassLabels.tsv");
	import("yago", "yagoTaxonomy.tsv");
	//import("yago","yagoDBpediaClasses.tsv");
	//import("yago","yagoDBpediaInstances.tsv");
	//import("yago","yagoMetaFacts.tsv");
	import("yago", "yagoImportantTypes.tsv");
	import("yago", "yagoSimpleTypes.tsv");
	import("yago", "yagoLiteralFacts.tsv");
	import("yago", "yagoFacts.tsv");
}

void import(const char* type, const char* filename) {

	clock_t start;
	double diff;
	//  start = clock();
	//  diff = ( std::clock() - start ) / (double)CLOCKS_PER_SEC;
	if (filename == 0)filename = type;
	if (eq(type, "all")) {
		importAll();
	} else if (eq(type, "csv")) {
		importCsv(filename);
	} else if (eq(type, "wordnet")) {
		importWordnet();
	} else if (eq(type, "names")) {
		importNames();
	} else if (eq(type, "images")) {
		importImages();
	} else if (eq(type, "wiki")) {
		importWikipedia();
	} else if (eq(type, "topic")) {
		importWikipedia();
	} else if (eq(type, "yago")) {
		if (eq(filename, "yago"))importAllYago();
		else if (contains(filename, "fact"))
			importFacts(filename, filename);
		else
			importYago(filename);
	} else if (contains(filename, "txt")) {
		importCsv(filename);
	} else if (contains(filename, "csv")) {
		importCsv(filename);
	} else if (contains(filename, "tsv")) {
		importCsv(filename);
	} else if (contains(filename, "xml")) {
		importXml(filename);
	} else {
		//if (!importFacts(filename, filename))
		printf("Unsupported file type %s %s", type, filename);
		importAll();
	}
	//  cout<<"nanoseconds "<< diff <<'\n';

	// importSqlite(filename);
	//    importNodes();
	//    importStatements();
}
/*root@h1642655:~/netbase# l facts/
actedIn           during              hasChild           hasISBN                hasRevenue       interestedIn    isSubstanceOf    subClassOf
bornIn            establishedOnDate   hasCurrency        hasLabor               hasSuccessor     inTimeZone      livesIn          subPropertyOf
bornOnDate        exports             hasDuration        hasMotto               hasTLD           inUnit          locatedIn        type
created           familyNameOf        hasEconomicGrowth  hasNominalGDP          hasUnemployment  isAffiliatedTo  madeCoverFor     until
createdOnDate     foundIn             hasExpenses        hasNumberOfPeople      hasUTCOffset     isCalled        means            using
dealsWith         givenNameOf         hasExport          hasOfficialLanguage    hasValue         isCitizenOf     musicalRole      worksAt
describes         graduatedFrom       hasGDPPPP          hasPages               hasWaterPart     isLeaderOf      originatesFrom   writtenInYear
diedIn            happenedIn          hasGini            hasPopulation          hasWebsite       isMarriedTo     participatedIn   wrote
diedOnDate        hasAcademicAdvisor  hasHDI             hasPopulationDensity   hasWeight        isMemberOf      politicianOf
directed          hasArea             hasHeight          hasPoverty             hasWonPrize      isNativeNameOf  produced
discovered        hasBudget           hasImdb            hasPredecessor         imports          isNumber        publishedOnDate
discoveredOnDate  hasCallingCode      hasImport          hasProduct             influences       isOfGenre       range
domain            hasCapital          hasInflation       hasProductionLanguage  inLanguage       isPartOf        since
 */
