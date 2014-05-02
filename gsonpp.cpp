#include <node.h>
#include <v8.h>
#include <iostream>       // std::cout
#include <stack>          // std::stack
#include <vector>         // std::list
#include <map>            // std::map
#include <fstream>
#include <string>

using namespace v8;

namespace gsonpp
{

uint32_t idx = 0;

std::string stringifyString(const std::string &str)
{
    std::string str_out = "\"";

    std::string::const_iterator iter = str.begin();
    while (iter != str.end())
    {
        char chr = *iter;

        if (chr == '"' || chr == '\\' || chr == '/')
        {
            str_out += '\\';
            str_out += chr;
        }
        else if (chr == '\b')
        {
            str_out += "\\b";
        }
        else if (chr == '\f')
        {
            str_out += "\\f";
        }
        else if (chr == '\n')
        {
            str_out += "\\n";
        }
        else if (chr == '\r')
        {
            str_out += "\\r";
        }
        else if (chr == '\t')
        {
            str_out += "\\t";
        }
        else
        {
            str_out += chr;
        }

        iter++;
    }

    str_out += "\"";
    return str_out;
}

inline bool isNode(Local<Value> v)
{
  return v->IsArray()||v->IsObject();
}

inline int oh(Local<Object> obj)
{
   Local<Value> v= obj->Get(String::NewSymbol("_"));

   if (v->IsUndefined())
     return -1;
   else
     return v->ToInteger()->NumberValue();
}

inline bool wasVisited(Local<Object> v)
{
   int hash = oh(v);

   if (hash<0)
   {
     v->Set(String::NewSymbol("_"),Integer::New(idx++));
     return false;
   } 
   else
     return true; 
}

inline void replaceQ(std::string input)
{
    size_t pos = 0;
    while (std::string::npos != (pos = input.find("\"", pos)))
    {
        input.replace(pos, 1, "\\\"", 2);
        pos += 2;
    }
}

std::string ts(Local<Value> v, bool force)
{
   std::string s= std::string(*v8::String::Utf8Value(v->ToString()));

   if (force || v->IsString())
     s= stringifyString(s);
   /*{
        replaceQ(s);
        s= "\""+s+"\"";
   }*/

   return s;
}

std::string ts(Local<Value> v)
{
   return ts(v,false);
}

void serializeObject(std::ofstream* out, Local<Object> obj)
{
   *out << "{";

   Local<Array> keys = obj->GetPropertyNames();
   uint32_t length = keys->Length();
   bool notfirst = false;

   for (uint32_t i=0 ; i<length ; ++i)
   {
       const Local<Value> key = keys->Get(i);
       const Local<Value> val = obj->Get(key);
       const std::string skey= ts(key,true);

       if (val->IsUndefined()||skey.compare("\"_\"")==0) continue;

       if (notfirst)
         *out << ","; 

       notfirst= true;

       if (isNode(val))
         *out << skey << ":{\"_\":" << oh(val->ToObject()) << "}"; 
       else
         *out << skey << ":" << ts(val);
   }

   *out << "}";
}

void serializeArray(std::ofstream* out, Local<Array> arr)
{
   *out << "[";

   uint32_t length = arr->Length();
   for (uint32_t i=0 ; i<length ; ++i)
   {
       const Local<Value> val = arr->Get(i);

       if (i>0)
         *out << ","; 

       if (isNode(val))
         *out << "{\"_\":" << oh(val->ToObject()) << "}"; 
       else
         *out << ts(val);
   }

   *out << "]";
}

void serializeNode(std::ofstream* out, Local<Value> node)
{
     if (node->IsArray())
       serializeArray(out,Local<Array>::Cast(node));
     else
       serializeObject(out,node->ToObject());
}

Handle<Value> Method(const Arguments& args)
{
  idx= 0;
  HandleScope scope;

  String::Utf8Value file(args[0]);
  Local<Value> graph(args[1]);

  std::stack< Local<Value> > rstack;
  std::vector< Local<Value> > nlist;

  rstack.push(graph);

  while (!rstack.empty())
  {
      Local<Value> current= rstack.top();
      rstack.pop();

      if (isNode(current))
      {
         Local<Object> obj= current->ToObject();

         if (wasVisited(obj)) continue;

         nlist.push_back(obj);

         Local<Array> keys = obj->GetPropertyNames();
         uint32_t length = keys->Length();
         for (uint32_t i=0 ; i<length ; ++i)
         {
            Local<Value> key = keys->Get(i);
            rstack.push(obj->Get(key));
         }
      }
  }

  uint32_t length = nlist.size();

  std::ofstream out(*file);

  out << "[";

  if (length>0)
  {
       
      serializeNode(&out,nlist[0]);

      for (uint32_t i=1 ; i<length ; ++i)
      {
         out << ",";
         serializeNode(&out,nlist[i]);
      }
  }
  
  out << "]";

  out.close();

  return scope.Close(Undefined());
}

}

void init(Handle<Object> exports) {
  exports->Set(String::NewSymbol("serialize"),
      FunctionTemplate::New(gsonpp::Method)->GetFunction());
}

NODE_MODULE(gsonpp, init)
