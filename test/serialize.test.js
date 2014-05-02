var should= require('chai').should(),
    assert= require('chai').assert,
    fs= require('fs'),
    GSON= require('gson'),
    gsonpp=  require('../build/Release/gsonpp');

var cir= function ()
{
    var a= { x: undefined, nome: 'Andrea' },
        e= { nome: 'Elena' };

    a.figlia= e;
    a.figlia2= e;
    e.papa= a;

    return [a,3,[e,a]];
};

describe('region',function ()
{
       beforeEach(function (done)
       {
           fs.unlink('a.gson',function () { done(); /* ignore err */ });
       });

       it('can serialize a circular object graph', function (done)
       {
           var gph= cir();

           gph[0][33]= 'age'; // test number keys

           gsonpp.serialize('a.gson',gph);

           var clone= GSON.parse(fs.readFileSync('a.gson','utf8'));

           clone[0].should.equal(clone[2][1]);
           clone[0].figlia.should.equal(clone[2][0]);

           done();
       });
});
