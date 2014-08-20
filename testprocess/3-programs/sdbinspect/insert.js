var db = new Sdb('sdbserver3',11810)
var start = 1000000;
var end = 10000000;
for (var i = start; i < end; i++)
{
   db.inspect_cs.inspect_cl.insert({id:i,'age':i+10,'content':'jjfdksal;jfkdl;ahgidopusairjeklwjqrkl;eusaiorueiojafkldjsalk;fjdklasfjiewqutihkfdhafdjsakf;jdsaklfjdskal','message':'hfdjsakfljdklsa;fjdkslaurioewuqiopjfkldjsakflhdsaifuioewuqirdihsafkdjsakfjdkslaj;fkdsajfdhgjdhsajfkhdsajkfdksjafkjdsa;lkghjkdhasiurpeiowqurijhdskaljfkdlsafhkjdsaurieuwqiojfkdjsalk;fdlsaurioewuqijfkldsafhdsatyipeuqioprhfhdjkafdkafdhapfiurioeuwqjfdkjas;lkhgdashfiduqproiuehfkdjaklfjdqopurieqwjrkhfdakljgioqpureiqrjhfdsajfkdsajf'});
   if (i%10000 == 0)
   {
      println("i = "+i);
   }
}
