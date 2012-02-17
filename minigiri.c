#include <libxml/parser.h>
#include <libxml/xpathInternals.h>
#include <ruby.h>

VALUE cNode, cNodeSet;

struct doc_data {
    VALUE doc;
};

#define DOC_RUBY_OBJECT(x) (((struct doc_data *)(x->_private))->doc)

static void
doc_dealloc(xmlDocPtr doc)
{
    free(doc->_private);
    xmlFreeDoc(doc);
}

static VALUE
doc_read_memory(VALUE klass, VALUE string)
{
    const char *c_buffer;
    int len;
    xmlDocPtr doc;
    VALUE rb_doc;
    struct doc_data *tuple;
    
    StringValue(string);
    c_buffer = RSTRING_PTR(string);
    len = (int)RSTRING_LEN(string);

    doc = xmlReadMemory(c_buffer, len, NULL, NULL, XML_PARSE_RECOVER);
    rb_doc = Data_Wrap_Struct(klass, 0, doc_dealloc, doc);

    tuple = (struct doc_data *)malloc(sizeof(struct doc_data));
    tuple->doc = rb_doc;
    doc->_private = tuple;

    return rb_doc;
}

static void
node_mark(xmlNodePtr node)
{
    rb_gc_mark(DOC_RUBY_OBJECT(node->doc));
}

static void
node_set_deallocate(xmlNodeSetPtr node_set)
{
    xmlFree(node_set->nodeTab);
    xmlFree(node_set);
}

static VALUE
node_set_length(VALUE self)
{
    xmlNodeSetPtr node_set;
    Data_Get_Struct(self, xmlNodeSet, node_set);
    return INT2NUM(node_set->nodeNr);
}

static VALUE
node_set_index_at(VALUE self, VALUE rb_offset)
{
    VALUE rb_node;
    xmlNodePtr node;
    xmlNodeSetPtr node_set;
    long offset = NUM2LONG(rb_offset);

    Data_Get_Struct(self, xmlNodeSet, node_set);

    if (offset >= node_set->nodeNr || offset < 0)
	return Qnil;
    
    node = node_set->nodeTab[offset];
    rb_node = Data_Wrap_Struct(cNode, node_mark, 0, node);

    return rb_node;
}

static VALUE
node_children(VALUE self)
{
    xmlNodePtr node, child;
    xmlNodeSetPtr set;
    VALUE document, rb_set;

    Data_Get_Struct(self, xmlNode, node);

    child = node->children;
    set = xmlXPathNodeSetCreate(child);

    document = DOC_RUBY_OBJECT(node->doc);

    if (child)
	child = child->next;
    
    while(child) {
	xmlXPathNodeSetAddUnique(set, child);
	child = child->next;
    }

    rb_set = Data_Wrap_Struct(cNodeSet, 0, node_set_deallocate, set);
    rb_iv_set(rb_set, "@document", document);

    return rb_set;
}

void
Init_minigiri()
{
    cNode = rb_define_class("Node", rb_cObject);
    rb_define_method(cNode, "children", node_children, 0);  

    VALUE cDocument = rb_define_class("Document", cNode);
    rb_define_singleton_method(cDocument, "read_memory", doc_read_memory, 1);

    cNodeSet = rb_define_class("NodeSet", rb_cObject);
    rb_define_method(cNodeSet, "length", node_set_length, 0);
    rb_define_method(cNodeSet, "[]", node_set_index_at, 1);

    xmlInitParser();  
}
