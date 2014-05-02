gsonpp
==========

Serialize huge circular object graphs to file,
without using extra V8 heap to do it.
Use [GSON](https://github.com/aaaristo/GSON) to deserialize.

```
npm install gsonpp gson
```

```javascript

var gsonpp= require('gsonpp');

var a= { name: 'Andrea' },
    e= { name: 'Elena' };
    
a.daughter= e;
e.dad= a;

gsonpp.serialize('a.gson',a);

var fs= require('fs'),
    GSON= require('gson');

console.log(GSON.parse(fs.readFileSync('a.gson','utf8')));
```
