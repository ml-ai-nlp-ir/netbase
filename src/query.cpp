#include "netbase.hpp"
#include "util.hpp"
#include "query.hpp"
#include "relations.hpp"

#include <cstdlib>
#include <string.h>

int resultLimit=100;// != lookuplimit
int* enqueued; // 'parents'
NodeVector EMPTY;


string select(string s) {
	if (contains(s, "select"))
		return "";
	else
		return "select * from ";
}
string fixQuery(string s) {
	s = s + string(" $");
	s = replace_all(s, "types of ", select(s)); //todo .*types of regex
	s = replace_all(s, "kinds of ", select(s));
	s = replace_all(s, "list of ", select(s));
	s = replace_all(s, "list ", select(s));
	s = replace_all(s, "species of ", select(s));
	s = replace_all(s, "all ", select(s)); // todo all fields of types that ...
	s = replace_all(s, "some ", select(s));
	s = replace_all(s, "different ", select(s));
	s = replace_all(s, "various ", select(s));
	s = replace_all(s, "the ", select(s));
	s = replace_all(s, "a ", select(s)); //limit 1
	s = replace_all(s, "these ", select(s));
	s = replace_all(s, "those ", select(s));
	s = replace_all(s, "an ", select(s));
	s = replace_all(s, "any ", select(s));
	s = replace_all(s, " are ", " ");
	s = replace_all(s, " is ", " ");
	s = replace_all(s, " with ", " where ");
	s = replace_all(s, " which ", " where ");
	s = replace_all(s, " who ", " where ");
	s = replace_all(s, " whose ", " where ");
	s = replace_all(s, " who is ", " where ");
	s = replace_all(s, " that ", " where ");
	s = replace_all(s, " in ", " where "); //location=           ...
	s = replace_all(s, " of ", " where "); // owner= or location=
	s = replace_all(s, " that ", " where ");
	s = replace_all(s, "ies ", "y "); //singular
	s = replace_all(s, "s ", " "); //singular
	s = replace_all(s, " $", ""); //trim hack
	return s;
}

Facet& findFacet(Query& q, Node* field) {
	if (q.facetMap.find(field) != q.facetMap.end())
		return *q.facetMap[field];
	Facet* f;
	for (int i = 0; i < q.facets.size(); i++) {
		f = q.facets[i];
		if (isA4(f->field, field)) {
			q.facetMap[field] = f;
			return *f;
		}
	}
	//    if q.autoFacet
	Facet& f2 = *new Facet(); // life cycle !?!
	f2.field = field;
	f2.type = get(field->kind);
	//    f2.values = new map<Node*, int>();
	q.facets.push_back(&f2);
	q.facetMap[field] = &f2;
	return f2;
}

int getFieldNr(Query& q, Node* predicate) {
	for (int i = 0; i < q.fields.size(); i++) {
		if (q.fields[i] == predicate)return i;
	}
	return -1;
}

void collectFacets(Query& q) {
	NodeVector all = q.instances;
	int nrFields = q.fields.size();
	for (int rowNr = 0; rowNr < all.size(); rowNr++) {
		Node* n = all[rowNr];
		q.values[n] = (NodeList) malloc(sizeof (Node*) * nrFields);
		for (int columnNr = 0; columnNr < n->statementCount; columnNr++) {
			Statement *s = getStatementNr(n, columnNr); //todo: ->next for efficiency
			if (s->Subject == n) {
				Node* predicate = s->Predicate;
				Node* value = s->Object;
				//                addHit(q,predicate)
				Facet& f = findFacet(q, predicate);
				f.hits++;
				map<Node*, int>& values = *(f.values);
				p(values.size());
				if (values.find(value) != values.end()) {// 4m!=4m !?!?
					values[value] = values[value] + 1;
					f.maxHits = max(f.maxHits, values[value]);
				}//                if (values.find(value)!=values.end())// no
					//                    values[value] = values[value] + 1;
					//                    f->values[value] = f->values->[value] + 1;
				else {
					values[value] = 1;
					if (f.maxHits == 0)f.maxHits = 1;
				}
				NodeList nl = q.values[n];
				int fieldNr = getFieldNr(q, predicate);
				if (fieldNr >= 0)
					nl[fieldNr] = value;
			}
		}

	}
}

string renderResults(Query& q) {
	NodeVector all = q.instances;
	int nrFields = q.fields.size();
	stringstream buffer; //(1000000, ' ');

	buffer << "<response>\n";
	buffer << "<lst name=\"responseHeader\">\n";
	buffer << "<int name=\"QTime\">0</int>\n";
	buffer << "<lst name=\"params\">\n";
	buffer << " <str name=\"q\">" << q.keyword->name << "</str>\n";
	buffer << " <str name=\"filters\">" << q.filters.size() << "</str>\n";
	buffer << " <str name=\"facets\">" << q.facets.size() << "</str>\n";
	buffer << " <int name=\"limit\">" << q.limit << "</int>\n";
	buffer << "</lst>\n";
	buffer << "</lst>\n";

	buffer << "<table name=\"response\" numFound=" << q.instances.size() << " start=" << q.start << " rows=" << q.hitsPerPage << ">\n";
	//    buffer << "<result name=\"response\" numFound=\"559\" start=\"0\">\n";

	// Field header
	buffer << "<th>\n";
	for (int columnNr = 0; columnNr < nrFields; columnNr++) {
		Node *f = q.fields[columnNr];
		string kind = "td"; // "str"
		buffer << " <" << kind << " field_id=" << f->id << " name=\"" << f->name << "\">" << f->name << "</" << kind << ">\n";
	}
	buffer << "</th>\n";

	// results
	for (int rowNr = 0; rowNr < all.size(); rowNr++) {
		Node* n = all[rowNr];
		//    buffer << "<doc>\n";
		buffer << "<tr>\n";
		buffer << "<td name=\"id\">" << n->id << "</td>\n";
		buffer << "<td name=\"context\">" << n->context << "</td>\n";
		buffer << "<td name=\"name\">" << n->name << "</td>\n";
		buffer << "<td name=\"kind\">" << n->kind << "</td>\n";
		buffer << "<td name=\"statementCount\">" << n->statementCount << "</td>\n";
		NodeList values = q.values[n];
		for (int columnNr = 0; columnNr < nrFields; columnNr++) {
			Node *f = q.fields[columnNr];
			Node *v = values[columnNr];
			if (!checkNode(v))continue;
			string kind = "td"; // "str"
			buffer << "<" << kind << " field_id=\"" << f->id << "\" value_id='" << v->id << "' name=\"" << f->name << "\" >" << v->name << "</" << kind << ">\n";
		}
		buffer << "</tr>\n";
		//    buffer << "</doc>\n";
	}
	buffer << "</table>\n";
	buffer << "</result>\n";

	// renderFacets
	buffer << "<lst name=\"facet_counts\">\n";
	//    buffer << "<lst name=\"facet_queries\"/>\n";
	buffer << "<lst name=\"facet_fields\">\n";
	for (int i = 0; i < q.facets.size(); i++) {
		Facet& f = *q.facets[i];
		buffer << "<lst name=\"" << f.field->name << "\">\n";
		map<Node*, int>::iterator it = f.values->begin();
		while (it != f.values->end()) {
			Node *slot = it->first; // same as (*it).first   (the key value)
			int count = it->second;
			buffer << "<int name=\"" << slot->name << "\">" << count << "</int>\n";
			it++;
		}
		buffer << "</lst>\n";
	}
	buffer << "</lst>\n";
	buffer << "</response>\n";
	return buffer.str();
}

NodeVector query(string s, int limit) {
	p(("Executing query "));
	ps(s);
	s = fixQuery(s);
	int li = s.find("limit");
	if (li > 0) {
		limit = atoi(s.substr(li + 5).c_str());
		s = s.substr(0, li);
	}
	return evaluate_sql(s, limit);
}

NodeVector query(Query& q) {
	NodeVector all = all_instances(q);
	for (int i = 0; i < q.keywords.size(); i++) // remove
		all = all_instances(q.keywords[i]); // bad

	q.instances = all;
	for (int i = 0; i < q.filters.size(); i++) {
		Statement* _filter = q.filters[i];
		clearAlgorithmHash();
		q.instances = filter(q, _filter);
	}
	collectFacets(q);
	return q.instances; // renderResults(q);
}

NodeVector nodeVector(vector<char*> v) {
	NodeVector nv;
	for (int i = 0; i < v.size(); i++) {
		nv.push_back(getThe(v[i]));
		;
		;
	}
	return nv;
}

// sentenceToStatement

Statement* parseSentence(string sentence, bool learn = false) {

	int recurse = 0;
	int limit = 5;
	sentence = replace_all(sentence, " a ", " ");
	sentence = replace_all(sentence, " the ", " ");
	vector<char*> matches = splitString(sentence, " ");
	if (matches.size() != 3) {
		ps("Currently only triples can be learned. If you have multiple_word nodes combine them with an underscore");
		return 0;
	}
	Node* subject = 0;
	Node* predicate = 0;
	Node* object = 0;
	for (int i = 0; i < matches.size(); i++) {
		string word = matches[i];
		//		word=stem(word);
		bool getPredicate = subject&&!predicate;
		int id=atoi(word.data());
		Node* node;
		if(id>0)
			node=get(id);
		else
			node= getAbstract(word.data());
//		Node* node = getThe(word); TODO getThe for relation!!!
		//		Node* abstract=getAbstract(f);// ,getPredicate?Verb:0
		//		NodeVector& instances=all_instances(abstract, recurse,limit);
		//		if(instances->size()<=0)word=abstract;
		//		else word= instances[0];
		if (!subject)subject = node;
		else if (!predicate)predicate = node; //stem(word);
		else if (!object)object = node; // John smells
	}
	if (!object)object = Any;
	if (!(subject && predicate))
		return 0;
	if (learn)
		return addStatement(subject, predicate, object, true);
	else
		return pattern(subject, predicate, object);
}

Statement *parseFilter(string s) {
	if (!contains(s, ".")&&!contains(s, "="))
		return parseSentence(s);

	Node* Subject = Any;
	Node* Predicate = Any;
	Node* Object = Any;
	// a.b=>(a,Member,b);
	// a.b=c => (a,b,c);
	if (s.find(".") > 0) {
		Subject = getThe(s.substr(0, s.find(".")));
		s = s.substr(s.find(".") + 1);
	}

	if (contains(s, " is ")) {
		Node* p = getThe(s.substr(0, s.find(" is ")));
		if (Subject != Any)Predicate = p;
		else {
			Subject = p;
			Predicate = Equals;
		}
		s = s.substr(s.find(" is ") + 4);
	}
	if (contains(s, "=")) {
		Node* p = getThe(s.substr(0, s.find("=")));
		if (Subject != Any)Predicate = p;
		else {
			Subject = p;
			Predicate = Equals;
		}
		s = s.substr(s.find("=") + 1);
	}
	if (contains(s, ">")) {
		Node* p = getThe(s.substr(0, s.find(">")));
		if (Subject != Any)Predicate = p;
		else {
			Subject = p;
			Predicate = Greater;
		}
		s = s.substr(s.find(">") + 1);
	}
	if (contains(s, "<")) {
		Node* p = getThe(s.substr(0, s.find("<")));
		if (Subject != Any)Predicate = p;
		else {
			Subject = p;
			Predicate = Less;
		}
		s = s.substr(s.find("<") + 1);
	}
	Object = getThe(s);
	return pattern(Subject, Predicate, Object);
}

Query parseQuery(string s, int limit) {
	Query q;
	static char fields[10000];
	static char type[10000];
	static char match[10000];
	int li = s.find("limit");
	if (li > 0) {
		limit = atoi(s.substr(li + 5).c_str());
		s = s.substr(0, li);
	}
	q.limit = limit;
	if ((int) s.find("from") < 0)
		s = string("select * from ") + s; // why (int) neccessary???
	sscanf(s.c_str(), "select %s from %s where %s", fields, type, match);
	char* where = " where ";
	char* match2 = (char*) contains(s.c_str(), where); // remove test.funny->.funny do later!
	if (match2) {
		match2 += 7; // wegen 'where' !!!! bad style
		p(match2);
	}
	q.keyword = getThe(type);
	//    q.keywords=nodeVector(splitString(type, ","));
	//    if(q.keywords.size()>0)
	//    q.keyword=q.keywords[0];
	q.fields = nodeVector(splitString(fields, ","));
	vector<char*> matches = splitString(replace_all(match, ",", " and "), " and ");
	for (int i = 0; i < matches.size(); i++) {
		string f = matches[i];
		Statement *s = parseFilter(f);
		q.filters.push_back(s);
	}
	return q;
}

string query2(string s, int limit) {
	s = fixQuery(s);
	Query q = parseQuery(s, limit);
	NodeVector all = query(q);
	string result = renderResults(q);
	return result;
}

// select *,fields from type where attributes match values

NodeVector evaluate_sql(string s, int limit = 20) {//,bool onlyResults=true){
	clearAlgorithmHash();
	NodeVector found;
	static char fields[10000];
	static char type[10000];
	static char where[10000];
	if(s.find("all ")==0)
		s=s.replace(0,4,"");
	if ((int) s.find("from") < 0)
		s = string("select * from ") + s; // why (int) neccessary???
	//	s=stem_singular(s)/
	//    s=replace_all(s,"ies","y");//singular
	ps(s);
	sscanf(s.c_str(), "select %s from %s where %[^\n]s", fields, type, where);
//	makeSingular(type);
	p(fields);
	p(type);
	p(where);

	int batch = limit * 50; // 200000000;//// dont limit wegen filter!! limit + 100;
	//    while (found.size() < limit && batch < 200000000) {
	//        batch = batch * 100; //=> max 9 runs
	// what if all==1000000 but limit==3?

	NodeVector all = all_instances(getAbstract(type));
	//  find_all(type, current_context, true, batch); // dont limit wegen filter!!
	pf("%d so far\n",all.size());
	if(where[0])
		found = filter(all, where); //fields

	//    add instance matches
	for (int i = 0; i < all.size(); i++) {
		if (found.size() >= limit)goto good;
		all.clear(); // hack to reset all_instances
		NodeVector all2 = all_instances((Node*) all[i], true, limit);
		if(where)
			mergeVectors(&found, filter(all2, where)); //fields)
	}
	batch = 200000000;

good:
	if (found.size() > limit)// dont deliver too much
		found.erase(found.begin() + limit, found.end());
	showNodes(found);
	return found;
}

// maria.freund=sven
// maria.freund => sven

Statement* evaluate(string data) {
	Statement* s = parseFilter(data); //Hypothesis Aim Objective supposition assumption
	// TODO MARK + CLEAR PATTERNS!!!
	Statement* result = findStatement(s->Subject, s->Predicate, s->Object);
	if (result)return result;
		//     Node* n=has(s->Subject,s->Predicate,s->Object);
		//     if(n)return n;
	else
		return addStatement(s->Subject, s->Predicate, s->Object,true);
}

Node* match(string data) {
	//    size_type t=data.find("[");
	const char* match = data.substr(data.find("[") + 1, data.find("]") - 1).c_str();
	const char* word = data.substr(0, data.find("[")).c_str();
	findMatch(getAbstract(word), match);
}

NodeVector exclude(NodeVector some, NodeVector less) {// bool keep destination unmodified=TRUE
	for (int i = some.size(); i > 0; --i) {
		if (contains(less, (Node*) some[i]))
			some.erase(some.begin() + i);
	}
	return some;
}

int queryContext = _pattern; // hypothesis

// TODO MARK + CLEAR PATTERNS!!!
Statement* pattern(Node* subject, Node* predicate, Node* object) {
	Statement *s = addStatement(subject, predicate, object, false); //todo mark (+reuse?) !
	if(checkStatement(s))
		s->context = _pattern;
	Node* pattern = reify(s);// why here?
	pattern->kind=_pattern;
	addStatement(pattern, is_a, Pattern, false);
	return s;// pattern?
}

NodeVector filter(NodeVector all, char* matches) {
	if(!matches||strlen(matches)==0|| all.size() == 0)return all;

	Query& q = *new Query();
	q.instances = all;
	static int calls = 0;
	// !! method static declaration will still increase with: !!!
	calls++;
	string buf; // Have a buffer string
	vector<string> tokens; // Create vector to hold our words
	string matchess = matches;
	stringstream ss(matchess); // Insert the string into a stream
	while (ss >> buf)
		tokens.push_back(buf);

	for (int y = all.size() - 1; y >= 0; --y) {
		Node* node = (Node *) all[y];
		if (!checkNode(node, 0, 0, 1))continue;
		if (!checkNode(node))continue;
		p("!++++++++++++++++++++++++++++\nfiltering node:");
		//		if (quiet)printf("%s ?",node->name);
		show(node);

		p("+++++++++++++++++++++++++++++\n");
		bool good = true;

		if (!quiet)
			printf("%d tokens in %s\n", tokens.size(), matches);

		// create match tree
		for (int i = 0; i < tokens.size(); i++) {
			string match = tokens[i];
			if (!quiet)
				printf("checking %s\n", match.c_str()); //Illegal instruction/operand if c_str missing!!!
			if (eq(match.c_str(), "and")) {
				p(">>and<<");
				ps(match);
				continue; // continue alone not breakable
			}
			//			printf("%s","checking "+match);
			if (match.find(node->name) == 0)
				match.replace(0, strlen(node->name), "");
			if (match.find(".") == 0)
				match.replace(0, 1, "");
			int comp = match.find("=");
			if (comp >= 0) {
				Node *s = getThe(match.substr(0, comp).data());
				Node *o = parseValue(match.substr(comp + 1).data());
				filter(q, pattern(s, Equals, o));
				continue;
			}
			comp = match.find(">");
			if (comp >= 0) {
				Node *s = getThe(match.substr(0, comp).data());
				Node *o = parseValue(match.substr(comp + 1).data());
				filter(q, pattern(s, Greater, o));
				continue;
			}
			comp = match.find("<");
			if (comp >= 0) {
				Node *s = getThe(match.substr(0, comp).data());
				Node *o = parseValue(match.substr(comp + 1).data());
				filter(q, pattern(s, Less, o));
				continue;
			}
			if (!findMatch(node, match.c_str())) {
				good = false;
				//			    break;
			}
		}
		// matches(node,match)
		if (!good) {
			all.erase(all.begin() + y);
			//            p("filtered!");
		} else
			p("passed!");
	}
	if (!quiet)
		printf("%d nodes passed\n", all.size());
	return all;
}


// filters: x[b] AND x.y=z OR x^z AND x.y>1 AND NOT x.y.z:[a,b] HOW?

//NodeVector filter(NodeVector all, Statement* filterTree) {
//    Query q;
//    q.instances=all;
//    return filter(all,filterTree);
//}
// must assign q.instances=filter(

NodeVector filter(Query& q, Statement* filterTree) {
	NodeVector all = q.instances;
	if(!checkStatement(filterTree))
		p("filterTree broken!\n");
	N subject = filterTree->Subject;
	N predicate = filterTree->Predicate;
	N object = filterTree->Object;

	if (predicate == And) {
		NV a = filter(q, subject);
		q.instances = a;
		NV b = filter(q, object); // a h ++
		return b; //intersect(a,b);
	} else if (predicate == Or) {
		NV a = filter(q, subject);
		NV b = filter(q, object);
		mergeVectors(&a, b);
		return a;
	} else if (predicate == Not) {// todo test
		NV a = all; //.clone()!
		NV b = filter(q, object);
		return exclude(a, b);
	}

	int size = all.size();
	for (int y = size - 1; y >= 0; --y) {
		Node* node = (Node *) all[y];
		if (node->kind == Abstract->id)continue;
		//        show(node);
		// cities where city->population->3999
		// cities where *->population->3999
		if (isA4(subject, node, q.recursion, q.semantic) || subject == Any) {
			if (!findStatement(node, predicate, object, q.recursion, q.semantic, false, q.predicatesemantic))
				all.erase(all.begin() + y);
		}//  cities where population->equals->3999
			//  match Berlin.population=3999
		else if (predicate == Equals) {
			if (!findStatement(node, subject, object, q.recursion, q.semantic, false, q.predicatesemantic))// &&! !findStatement(node, subject, object)
				all.erase(all.begin() + y);
			else {
				//                ps("found");
				//                show(node);
				p(y);
			}

		} else {// free filters:  // cities where population->equals->3999
			Node* found = has(node, filterTree, q.recursion, q.semantic, false, q.predicatesemantic);
			if (!found)
				found = has(node, filterTree, 1, true, false, true);
			if (!found) {
				//                ps("mismatch");
				//                showStatement(filterTree);
				//                show(node);
				all.erase(all.begin() + y);
			} else {
				//                ps("found");
				//                show(node);
				p(y);
			}

			//            N n = has(node, subject);
			//            if (!findStatement(n, predicate, object))
			//                all.erase(all.begin() + y);
		}
	}
	return all;
}

NodeVector filter(Query& q, Node* _filter) {
	NodeVector all = q.instances;
	for (int y = all.size() - 1; y >= 0; --y) {
		Node* node = (Node *) all[y];
		if (isStatement(_filter)) {
			all = filter(q, isStatement(_filter));
		} else if (!findStatement(node, Any, _filter, q.recursion, q.semantic, false, q.predicatesemantic))
			all.erase(all.begin() + y);
	}
	return all;
}



//char* all="*";

void enqueueClass(Query& q, queue<Node*>& classQueue, Node* c) {
	if (!checkNode(c))return;
	//    if(!contains(classQueue | q.classes, c)){// cyclesave!
	classQueue.push(c);
	q.classes.push_back(c);
	//    }
}

NodeVector& nodesOfDirectType(int kind){
	NodeVector all;
	for (int i = 0; i < currentContext()->nodeCount; i++) {
		Node* n = &currentContext()->nodes[i];
		if(checkNode(n,i,false,false)&&n->kind==kind)
			all.push_back(n);
	}
	return all;
}
NodeVector& all_instances(Query& q) {
	queue<Node*> classQueue;
	p(q);
	enqueueClass(q, classQueue, q.keyword);
	q.instances.push_back(q.keyword); // really?? joain. joa!
	for (int i = 0; i < q.keywords.size(); i++)
//		if(!classQueue.contains)
		classQueue.push(q.keywords[i]);
	Node* c;
	int j = 0;
	while (classQueue.size() > 0) {
		c = classQueue.front();
		classQueue.pop();
		if (yetvisited[c])continue;
		yetvisited[c] = true;
		if (!checkNode(c))continue;
		pf("all %d %s\n",c->id,c->name);
		for (int i = 0; i < c->statementCount; i++) {
			Statement* s = getStatementNr(c, i);
			if (!checkStatement(s))continue;
			if (q.instances.size() >= resultLimit)
				return q.instances;
			if ( j++ >= q.lookuplimit){
//				p("lookuplimit reached");
				break;
			}
//			showStatement(s);
			if (s->Subject == q.keyword) {
				if(contains(q.instances,s->Object))continue;
				if (isA4(s->Predicate, Instance, false, false)){
					q.instances.push_back(s->Object);
//					if(q.depth>0)// q.depth-- >0 ==7  ? todo !
						enqueueClass(q,classQueue,s->Object);
					continue; // found one!
				}
				//            if (isA4(s->Predicate, Instance, false, false))if (!contains(q.instances, s->Object))q.instances.push_back(s->Object);
				if (isA4(s->Predicate, SubClass, false, false))enqueueClass(q, classQueue, s->Object);
				if (isA4(s->Predicate, Plural, false, false))enqueueClass(q, classQueue, s->Object);
				if (isA4(s->Predicate, Synonym, false, false))enqueueClass(q, classQueue, s->Object);
			} else {
				if(contains(q.instances,s->Subject))continue;
				if (isA4(s->Predicate, Type, false, false)){
					q.instances.push_back(s->Subject);
					pf("%d %s\n",s->Subject->id,s->Subject->name);
//					if(q.depth>0)
					enqueueClass(q,classQueue,s->Subject);
					continue; // found one!
				}
				if (isA4(s->Predicate, SuperClass, false, false))enqueueClass(q, classQueue, s->Subject);
				if (isA4(s->Predicate, Plural, false, false))enqueueClass(q, classQueue, s->Subject);
				if (isA4(s->Predicate, Synonym, false, false))enqueueClass(q, classQueue, s->Subject);
			}
		}
	}
	NodeVector allOfType0=nodesOfDirectType(q.keyword->id);
	addRange(q.instances,allOfType0,false);
	//    q.instances = addRange(q.instances, q.classes); //nee
	return q.instances;
}


// todo: EXCLUDING classes and direct instances on demand!
NodeVector& all_instances(Node* type) {
	clearAlgorithmHash();

//	NodeVector all=instanceFilter(type);
//	return all;//  all_instances(getQuery(type));
	// COMPARE: !!
	return all_instances(type,true,resultLimit);
//	return recurseFilter(type,true,resultLimit,instanceFilter);
}

NodeVector& all_instances(Node* type, int recurse, int max,bool includeClasses) {
	static NodeVector& all = *new NodeVector; // empty before!
	if (type == 0) {
		all.clear(); // hack!!!
		return EMPTY;
	}
	if (recurse > 0)recurse = recurse + 2; // recurse++; dont descend too deep! todo: as iterator anyways!
	if (yetvisited[type])
		return all;
	yetvisited[type] = true;
	if (recurse > maxRecursions)return all;
	runs++;
	if (runs > maxNodes)return all; // no infinite loops!
	NodeVector subtypes;
	// todo : via instanceFilter see below
	for (int i = 0; i < type->statementCount; i++) {
		Statement* s = getStatementNr(type, i);
		if (!checkStatement(s))continue;
		if(s->Predicate==type)continue;// NO Predicate matches!!
//		showStatement(s);
		//    	po
		if (s->Subject == type) {
			if(isAbstract(type)&&isA4(s->Predicate, Instance, false, false))if (!contains(subtypes, s->Object))subtypes.push_back(s->Object);
			if (!isAbstract(type)&&isA4(s->Predicate, Instance, false, false))if (!contains(all, s->Object))all.push_back(s->Object);
			if (isA4(s->Predicate, SubClass, false, false))if (!contains(subtypes, s->Object))subtypes.push_back(s->Object);
			if (isA4(s->Predicate, Plural, false, false))if (!contains(subtypes, s->Object))subtypes.push_back(s->Object);
			if (isA4(s->Predicate, Synonym, false, false))if (!contains(subtypes, s->Object))subtypes.push_back(s->Object);
		} else {
			if (isA4(s->Predicate, Type, false, false))all.push_back(s->Subject);
			if (isA4(s->Predicate, SuperClass, false, false))subtypes.push_back(s->Subject);
			if (isA4(s->Predicate, Plural, false, false))if (!contains(subtypes, s->Subject))subtypes.push_back(s->Subject);
			if (isA4(s->Predicate, Synonym, false, false))if (!contains(subtypes, s->Subject))subtypes.push_back(s->Subject);
		}
		if (all.size() >= max || subtypes.size() >= max)break;
	}

//	subtypes.push_back(type); NOT AGAIN
	if (recurse)
		for (int i = 0; i < subtypes.size(); i++) {// subtypes
			if (subtypes.size() >= max)return all;
			Node* x=(Node*) subtypes[i];
			pf("all %d %s\n",x->id,x->name);
			if(!checkNode(x))continue;// how??
			NodeVector more=instanceFilter(x);//   all_instances(x, recurse-1, max);
			mergeVectors(&all, more);
		}
	subtypes.push_back(type);
	if(includeClasses)
	mergeVectors(&all, subtypes);// ja?
	return all;
}

NodeVector& recurseFilter(Node* type, int recurse, int max,NodeVector(*edgeFilter)(Node*, NodeQueue*)) {
	static NodeVector& all = *new NodeVector; // empty before!
	if (type == 0) {
		all.clear(); // hack!!!
		return EMPTY;
	}
	if (recurse > 0)recurse = recurse + 2; // recurse++; dont descend too deep! todo: as iterator anyways!
	if (yetvisited[type])
		return all;
	yetvisited[type] = true;
	if (recurse > maxRecursions)return all;
	runs++;
	if (runs > maxNodes)return all; // no infinite loops!

	NodeVector more=edgeFilter(type,null);
	mergeVectors(&all, more);

		for (int i = 0; i < more.size(); i++) {// subtypes
			if (all.size() >= max)return all;
			Node* x=(Node*) more[i];
			if(!checkNode(x))continue;// how??
			pf("all %d %s\n",x->id,x->name);
			recurseFilter(x,recurse, max,edgeFilter);// recurse++ above OK, adds to 'all'
//			mergeVectors(all,recurseFilter(x,recurse, max,edgeFilter));// recurse++ above
		}
	all.push_back(type);
	return all;
}


Query& getQuery(Node* keyword) {
	Query& q = *new Query();
	q.keywords.clear(); // =new NodeVector();
	q.keyword = keyword;
	q.classes.clear();
	//    q.recursion = recurse;
	q.depth = maxRecursions; //ja?
	//    q.limit = max;
	q.runs = 0;
	return q;
}


void clearAlgorithmHash(bool all) {
			runs = 0;
			yetvisited.erase(yetvisited.begin(), yetvisited.end()); //erase yetvisited
			yetvisited.clear();
//			only if low mem!!~:::
			yetvisitedIsA.erase(yetvisitedIsA.begin(), yetvisitedIsA.end()); //erase yetvisited
			yetvisitedIsA.clear();
			all_instances(0, 0); //hack! //			all.clear();
			recurseFilter(0,0,0,0);
}

NodeVector find_all(char* name, int context, int recurse, int limit) {
	clearAlgorithmHash();
	// if(context> -1)search subcontexts also?
	// if(context==-1)search all
	// if(context==-2)context=current_context;
	Context* c = getContext(context);
	NodeVector all;
	if (recurse > 0)recurse++;
	if (recurse > maxRecursions)return all;

	int max = min((long)c->nodeCount, maxNodes);
	all.push_back(getAbstract(name));

	//    for (int i = 0; i < max; i++) {// inefficient^2 use word->instance->... instead
	//        Node* n = &c->nodes[i];
	//        if (n == null || n->name == null || name == null)
	//            continue;
	//        if (eq(n->name, name)) {
	//            all.push_back(n);
	//            if (all.size() > limit)return all;
	//            // printf("found node %s in context %d\n",word,context);						//
	//            // show(n);
	//        }
	//    }
	int alle = all.size();
	if (recurse)
		for (int i = 0; i < alle; i++) {
			all.clear(); // hack to reset all_instances
			Node* node = (Node*) all[i];
			NodeVector& neu = all_instances(node, true, limit);
			mergeVectors(&all, neu);
		}
	return all;
}

Node* findMatch(Node* n, const char* match) {//
	char a[10000];
	char b[10000];
	char c[10000];
	b[0] = 0; //otherwise reused!?!
	c[0] = 0;
	bool matching; // leider immer !? --
	string smatch = replace_all(match, "=", " =");
	matching = sscanf(smatch.c_str(), "%s =%s", a, b);
	if (!quiet)
		printf("test findMatch %s %s\n", n->name, match);
	flush();
	if (!matching)
		p("scanf not matching");
	p(a);
	p(b);
	if (b != 0 && !eq(b, "")) {
		p("n[a=b]");
		if (!quiet)
			printf("show(findStatement(%s,%s,%s))", n->name, a, b);
		Statement* statement = findStatement(n, a, b);
		if (statement) {
			showStatement(statement);
			return n; //statement->Subject;
		} else
			return null;
	} else {
		p("n[match]");
		if (!quiet)
			printf("show(findMember(%s,%s))", n->name, a);
		Node* member = findMember(n, a);
		//		show(member);
		if (member)return n; //findMember(n,a);
		else return 0;
	}
	return 0;
}

int countInstances(Node* node) {
	int j = instanceFilter(node).size();
	int i = all_instances(node).size();
	set(node, the("direct instance count"), value(0, j));
	set(node, the("total instance count"), value(0, i));
	show(node, false);
	ps("statement count");
	p(node->statementCount);
	ps("direct instance count");
	p(i);
	ps("total instance count");
	p(j);
	return i;
}

NodeVector instanceFilter(Node* subject, NodeQueue* queue) {
	NodeVector all;

	int i = 0;
	Statement* s = 0;
	while(i++<resultLimit*2 && (s=nextStatement(subject,s,false))){// true !!!!
		bool subjectMatch = (s->Subject == subject || subject == Any);
		bool predicateMatch = (s->Predicate == Instance);

		bool subjectMatchReverse = s->Object == subject;
		bool predicateMatchReverse = s->Predicate == Type; // || inverse

		if (queue) {
			if (subjectMatch)enqueue(subject, s->Object, queue);
			if (subjectMatchReverse)enqueue(subject, s->Subject, queue);
		} else {
			if (subjectMatch && predicateMatch)all.push_back(s->Object);
			if (subjectMatchReverse && predicateMatchReverse)all.push_back(s->Subject);
		}
	}
	return all;
}

NodeVector memberFilter(Node* subject, NodeQueue * queue) {
	NodeVector all;
	int i = 0;
	Statement* s = 0;
	while(i++<1000 && (s=nextStatement(subject,s,false))){// true !!!!
		bool subjectMatch = (s->Subject == subject || subject == Any);
		bool predicateMatch = (s->Predicate == Member);
		predicateMatch = predicateMatch || s->Predicate == Part;
		predicateMatch = predicateMatch || s->Predicate == Attribute;
		predicateMatch = predicateMatch || s->Predicate == Substance;
		predicateMatch = predicateMatch || s->Predicate == Active;
		predicateMatch = predicateMatch || s->Predicate == To;
		predicateMatch = predicateMatch || s->Predicate == For;
		// include Parents!
		predicateMatch = predicateMatch || s->Predicate == Type;
		predicateMatch = predicateMatch || s->Predicate == SuperClass;
		predicateMatch = predicateMatch || s->Predicate == Synonym;
		predicateMatch = predicateMatch || s->Predicate == Plural;
		predicateMatch = predicateMatch || s->Predicate->id == _MEMBER_DOMAIN_CATEGORY;
		predicateMatch = predicateMatch || s->Predicate->id == _MEMBER_DOMAIN_REGION;
		predicateMatch = predicateMatch || s->Predicate->id == _MEMBER_DOMAIN_USAGE;


		bool subjectMatchReverse = s->Object == subject;
		bool predicateMatchReverse = s->Predicate == Owner; // || inverse
		predicateMatchReverse = predicateMatchReverse || s->Predicate == By;
		predicateMatchReverse = predicateMatchReverse || s->Predicate == From;
		predicateMatchReverse = predicateMatchReverse || s->Predicate == UsageContext;
		predicateMatchReverse = predicateMatchReverse || s->Predicate->id == _DOMAIN_CATEGORY;
		predicateMatchReverse = predicateMatchReverse || s->Predicate->id == _DOMAIN_REGION;
		predicateMatchReverse = predicateMatchReverse || s->Predicate->id == _DOMAIN_USAGE;
		// MORE :
		predicateMatchReverse = predicateMatchReverse || s->Predicate == Plural;
		predicateMatchReverse = predicateMatchReverse || s->Predicate == Synonym;
		predicateMatchReverse = predicateMatchReverse || s->Predicate == SubClass;


		if (queue) {
			if (subjectMatch)enqueue(subject, s->Object, queue);
			if (subjectMatchReverse)enqueue(subject, s->Subject, queue);
		} else {
			if (subjectMatch && predicateMatch)all.push_back(s->Object);
			if (subjectMatchReverse && predicateMatchReverse)all.push_back(s->Subject);
			if (subjectMatch && s->Predicate->id>1000)all.push_back(s->Object);
		}
	}
	if(queue)// already enqueued
		return EMPTY; //hack
	else
		return all;
}

NodeVector parentFilter(Node* subject, NodeQueue * queue) {
	NodeVector all;
	int i = 0;
	Statement* s = 0;
	while(i++<1000 && (s=nextStatement(subject,s,false))){// true !!!!
		if(s->context==_pattern)continue;
//		if(s->Predicate==Instance && !eq(s->Object->name,subject->name) )break;// needs ORDER! IS THE FIRST!!
//		if(s->Predicate==Type&&s->Object==subject)break;// todo PUT TO END TOO!!!
		bool subjectMatch = (s->Subject == subject || subject == Any);
		bool predicateMatch = (s->Predicate == Type);
		predicateMatch = predicateMatch || s->Predicate == SuperClass;
		predicateMatch = predicateMatch || s->Predicate == Synonym;
		predicateMatch = predicateMatch || s->Predicate == Plural;

		bool subjectMatchReverse = s->Object == subject;
		bool predicateMatchReverse = s->Predicate == Instance; // || inverse
		predicateMatchReverse = predicateMatchReverse || s->Predicate == Plural;
		predicateMatchReverse = predicateMatchReverse || s->Predicate == Synonym;
		predicateMatchReverse = predicateMatchReverse || s->Predicate == SubClass;

		if (queue) {
			if (subjectMatch&& predicateMatch)enqueue(subject, s->Object, queue);
			if (subjectMatchReverse&& predicateMatchReverse)enqueue(subject, s->Subject, queue);
		} else {
			if (subjectMatch && predicateMatch)all.push_back(s->Object);
			if (subjectMatchReverse && predicateMatchReverse)all.push_back(s->Subject);
		}
	}
	if(queue)// already enqueued
		return EMPTY; //hack
	else
		return all;
}


// todo : memory LEAK NodeVector ?
// todo : enqueue instances?

NodeVector anyFilter(Node* subject, NodeQueue * queue, bool includeRelations) {
	if (!includeRelations && subject->id < 1000)return EMPTY;
	NodeVector all;
	int i = 0;
	Statement* s = 0;
	while(i++<10000 && (s=nextStatement(subject,s,false))){
		if (!checkStatement(s)) {
			badCount++;
			continue;
		}

		bool subjectMatch = (s->Subject == subject || subject == Any);
		bool subjectMatchReverse = s->Object == subject;
		if (queue) {
			if (subjectMatch)enqueue(subject, s->Object, queue);
			if (subjectMatchReverse)enqueue(subject, s->Subject, queue);
		} else {
			if (subjectMatch)all.push_back(s->Object);
			if (subjectMatchReverse)all.push_back(s->Subject);
		}
	}
	if(queue)// already enqueued
		return EMPTY; //hack
	else
		return all;
}

NodeVector anyFilterNoKinds(Node* subject, NodeQueue * queue) {
	return anyFilter(subject, queue, false);
}
NodeVector anyFilterRandom(Node* subject, NodeQueue * queue) {
	return anyFilter(subject, queue, true);
}

NodeVector reconstructPath(Node* from, Node * to) {
	Node* current = to;
	NodeVector all;
	bool ok=true;
//	p("++++++++ FOUND PATH ++++++++++++++");
	//	enqueued[from->id]=0;// terminate
	while (current && current != from) {
		all.push_back(current);
		int id = enqueued[current->id];
		if (id <= 0){ok=false;break;}
		current = get(id);
//		show(current,false);
		if (contains(all, current))break;//LOOOOOP
	}
//	if(!ok){
//	current = from;
//	while (current && current != to) {
//		all.push_back(current);
//		int id = enqueued[current->id];
//		if (id <= 0)break;
//		current = get(id);
//		if (contains(all, current))break;//LOOOOOP
//	}
//	}
	all.push_back(from); // done
//	if(all.size()>2)
	free(enqueued);
	return all;
}

bool enqueue(Node* current, Node* d, NodeQueue * q) {
	if (enqueued[d->id])return false; // already done -> continue;
//	printf("? %d %s\n",d->id, d->name);
	// todo if d==to stop here!
	enqueued[d->id] = current->id;
	q->push(d);
	runs++;
	return true;
}

NodeVector findPath(Node* fro, Node* to, NodeVector(*edgeFilter)(Node*, NodeQueue*)) {
	//    map<Node*, Node*>enqueued;
	enqueued = (int*) malloc(currentContext()->nodeCount * sizeof (int));
	NodeQueue q;
	if (enqueued == 0) {
		p("out of memory for findPath");
		throw "out of memory for findPath";
	}
	memset(enqueued, 0, currentContext()->nodeCount * sizeof (int));

	ps("LOAD!");
	q.push(fro);
	runs=0;

	// NOT neccessary for anyPath , ...
	NodeVector instances;
	if(edgeFilter!=anyFilterNoKinds && edgeFilter!=instanceFilter &&edgeFilter!=anyFilterRandom) // && edgeFilter!=
		instances = all_instances(fro);
	for (int i = 0; i < instances.size(); i++) {
		Node* d = instances[i];
		enqueued[d->id] = fro->id;
		q.push(d);
	}
	pf("TO %d %s\n",to->id,to->name);
//	p(to);
	p("GO!");

	Node* current;

	while (current = q.front()) {
		if (q.empty())break;
		q.pop();
//		pf("?? %d %s\n",current->id,current->name);
		if (to == current)
			return reconstructPath(fro, to);
		if (!checkNode(current, 0, true))
			continue;
		NodeVector all = edgeFilter(current, &q);
		if (all != EMPTY)// no queue
			for (int i = 0; i < all.size(); i++) {
				Node* d = (Node*) all[i];
				if (to == current)return reconstructPath(fro, to);// shortcut
				enqueue(current, d, &q);
			}
	}
	pf("Touched nodes: %d\n",runs);
	return EMPTY;// NONE FOUND!
//	return reconstructPath(fro, to);
}

NodeVector memberPath(Node* from, Node * to) {
	NodeVector all = findPath(from, to, memberFilter);
	showNodes(all, false, true);
	return all;
}

NodeVector parentPath(Node* from, Node * to) {
	NodeVector all = findPath(from, to, parentFilter);
	if(all.size()>0)p("+++++++++++++++++++++++++++++++++");
	showNodes(all, false, true);
	return all;
}

NodeVector shortestPath(Node* from, Node * to) {
	NodeVector all = findPath(from, to, anyFilterNoKinds);
	if(all.size()==0)all = findPath(from, to, anyFilterRandom);
	showNodes(all, false, true);
	return all;
}