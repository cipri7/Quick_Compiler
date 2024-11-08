# program de test

var i:int;
i=0;    # iterator
while(i<10)
    puti(max(i,5));
    i=i+1;
    end

function max(x:int, y:int):int
    if(x<y)
        return y;
        else
        return x;
        end
    end
    
puts("PI=");
putr(3.14159);
