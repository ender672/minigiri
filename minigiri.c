#include <libxml/parser.h>
#include <ruby.h>

VALUE cNode;

struct doc_data {
    VALUE doc;
    VALUE node_cache;
};

#define DOC_RUBY_OBJECT(x) (((struct doc_data *)(x->_private))->doc)
#define DOC_NODE_CACHE(x) (((struct doc_data *)(x->_private))->node_cache)

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
    VALUE rb_doc, cache;
    struct doc_data *tuple;
    
    StringValue(string);
    c_buffer = RSTRING_PTR(string);
    len = (int)RSTRING_LEN(string);

    doc = xmlReadMemory(c_buffer, len, NULL, NULL, XML_PARSE_RECOVER);
    rb_doc = Data_Wrap_Struct(klass, 0, doc_dealloc, doc);

    cache = rb_ary_new();
    rb_iv_set(rb_doc, "@node_cache", cache);

    tuple = (struct doc_data *)malloc(sizeof(struct doc_data));
    tuple->doc = rb_doc;
    tuple->node_cache = cache;
    doc->_private = tuple;

    return rb_doc;
}

static void
node_mark(xmlNodePtr node)
{
    rb_gc_mark(DOC_RUBY_OBJECT(node->doc));
}

static VALUE
node_each(VALUE self)
{
    VALUE node_cache, rb_child;
    xmlNodePtr node, child;

    Data_Get_Struct(self, xmlNode, node);

    child = node->children;
    node_cache = DOC_NODE_CACHE(node->doc);
    
    while (child) {
	rb_child = Data_Wrap_Struct(cNode, node_mark, 0, child);
	rb_ary_push(node_cache, rb_child);
	rb_yield(rb_child);
	child = child->next;
    }

    return self;
}

void
Init_minigiri()
{
    cNode = rb_define_class("Node", rb_cObject);
    rb_define_method(cNode, "each", node_each, 0);
    rb_include_module(cNode, rb_mEnumerable);

    VALUE cDocument = rb_define_class("Document", cNode);
    rb_define_singleton_method(cDocument, "read_memory", doc_read_memory, 1);

    xmlInitParser();  
}
