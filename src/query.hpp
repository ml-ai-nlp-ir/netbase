/*
 * File:   query.h
 * Author: me
 *
 * Created on March 28, 2012, 6:46 PM
 */
#include "util.hpp"

#ifndef QUERY_H
#define	QUERY_H

//string render_query(Query& q);// renderResults!
Query& getQuery(Node* keyword);
NodeVector query(Query& q);
NodeVector query(string s, int limit=resultLimit);
Node* match(string data);
Node* findMatch(Node* n, const char* match);
Statement* evaluate(string data,bool learn=false);
int countInstances(Node* node);
//extern "C"
string query2(string s,int limit=resultLimit);
Query parseQuery(string s,int limit=resultLimit);
Statement* pattern(Node* subject, Node* predicate, Node* object);
void clearAlgorithmHash(bool all=false);
NodeVector exclude(NodeVector some, NodeVector less);
NodeVector evaluate_sql(string s, int limit) ;
NodeVector find_all(cchar* name, int context=1, int recurse=0, int limit=resultLimit);// all city
NodeSet findAll(Node* fro, NodeVector(*edgeFilter)(Node*, NodeQueue*,int*));// all subclassFilter etc
NodeVector& all_instances(Node* type, int recurse, int max= resultLimit,bool includeClasses=true);
//NodeVector& allInstances(Node* type);
NodeVector allInstances(Node* type, int recurse=false, int max=100, bool includeClasses=false);
NodeVector& all_instances(Query& q);

NodeVector& recurseFilter(Node* type, int recurse, int max,NodeVector(*edgeFilter)(Node*, NodeQueue*,int*));

NodeVector filter(Query& q, Node* _filter,int limit=0);
NodeVector filter(Query& q, Statement* filterTree,int limit=0);
NodeVector filter(NodeVector all, cchar* matches);
NodeVector filter(NodeVector all, Node* filterTree);
//NodeVector filter(NodeVector all, Statement* filterTree);

bool enqueue(Node* current, Node* d, NodeQueue* q,int* enqueued);
//typedef (NodeVector (*edgeFilter)(Node*)) NodeFilter;
NodeVector findPath(Node* fro, Node* to,NodeVector (*edgeFilter)(Node*,NodeQueue*,int*) );
NodeVector parentFilter(Node* subject,NodeQueue* queue=null,int* enqueued=null);
NodeVector subclassFilter(Node* subject, NodeQueue * queue,int* enqueued=null);
NodeVector memberFilter(Node* subject,NodeQueue* queue=null,int* enqueued=null);
NodeVector hasFilter(Node* subject,NodeQueue* queue=null,int* enqueued=null);
NodeVector childFilter(Node* subject,NodeQueue* queue=null,int* enqueued=null);
NodeVector ownerFilter(Node* subject,NodeQueue* queue=null,int* enqueued=null);
NodeVector anyFilter(Node* subject,NodeQueue* queue=null,bool includeRelations=true,int* enqueued=null);
NodeVector instanceFilter(Node* subject,NodeQueue* queue=null,int* enqueued=null);// chage all + edgeFilter!! for , int max= lookupLimit);// resultLimit
NodeVector relationsFilter(Node* subject, NodeQueue * queue=null,int* enqueued=null);

NodeVector parseProperties(const char *data);
NodeVector update(cchar* query);
NodeVector nodeVectorWrap(Node* n);
NodeVector nodeVectorWrap(Statement* n);

//NodeVector parentFilter(Node* subject);
//NodeVector memberFilter(Node* subject);
//NodeVector hasFilter(Node* subject);
//NodeVector childFilter(Node* subject);
//NodeVector ownerFilter(Node* subject);
//NodeVector anyFilter(Node* subject);
//NodeVector instanceFilter(Node* subject);
NodeVector nodeSetToNodeVector(NodeSet& input);
NodeVector setToVector(NodeSet& input);
//N loadBlacklist(bool reload=false);
map<int,bool> loadBlacklist(bool reload=false);// int: wordhash
NV findEntites(cchar* query);
N getClass(N n);
N getTopic(N n);// highest
NV getTopics(N entity);
NV getTopics(NV entities);

NodeVector shortestPath(Node* from,Node* to );// any
NodeVector parentPath(Node* from, Node* to);
NodeVector memberPath(Node* from, Node* to);
void sortNodes(NodeVector& all);

Node* findRelation(Node* from, Node* to);
Node* findProperty(Node* n , const char* m,bool allowInverse=true,int limit=0);
NodeVector findProperties(const char* n, const char* m,bool allowInverse=true);
NodeVector findProperties(Node* n, const char* m,bool allowInverse=true);
NodeVector findProperties(Node* n , Node* m,bool allowInverse=true);
//NodeVector* findWords(int context, const char* word, bool first= false,bool containsWord=false);
NodeVector* findWordsByName(int context, const char* word, bool first= false,bool containsWord=false);
NodeVector* findAllWords(const char* word);
//NodeVector find_all(char* name, int context = current_context, int recurse = false, int limit = defaultLimit);
extern "C"
Statement* findStatement(int subject, int predicate, int object, int recurse = false, bool semantic = useSemantics, bool symmetric = false,bool semanticPredicate=false, bool matchName=false);
Statement* findStatement(Node* subject, Node* predicate, Node* object, int recurse = false, bool semantic = useSemantics, bool symmetric = false,bool semanticPredicate=useSemantics, bool matchName=false,int limit=lookupLimit);// 
Statement* findStatement(Node* n, string predicate, string object, int recurse = false, bool semantic = useSemantics, bool symmetric = false);

void setValue(Node* node, Node* property, Node* value);
//void value(
//void setKind
bool isA4(Node* n, string match, int recurse = false, bool semantic = false);
bool isA4(Node* n, Node* match, int recurse = 0, bool semantic = false, bool matchName=false);
extern "C"
bool isA(Node* fro, Node* to);
bool filterWikiType(int object);
Statement * findRelations(Node* from, Node * to);
Statement* parseSentence(string sentence, bool learn = false);

#endif	/* QUERY_H */
