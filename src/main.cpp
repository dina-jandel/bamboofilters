// ovaj kod napisala je clanica tima: Dina Janđel
#include <iostream>   
#include <string>     

/*
    deklaracija funkcije runExperiments()
    funkcija pokrece testiranje implementiranog Bamboo filtera i sprema rezultate u datoteku

    parametri:
    outputFile -> ime izlazne datoteke u koju se spremaju rezultati
    fastaFile  -> opcionalna FASTA datoteka s DNA sekvencama 

    ako fastaFile nije proslijeden:
    koristi se generirana random DNA sekvenca
*/
void runExperiments(
    const std::string &outputFile,
    const std::string &fastaFile = "");

int main(
    int argc,
    char *argv[])
{
    /*
        primjer poziva: 
        ./bamboo_project --file test.fasta --out rez.txt
        argc: 5
        argv:
        argv[0] -> ime programa
        argv[1] -> "--file"
        argv[2] -> "test.fasta"
        argv[3] -> "--out"
        argv[4] -> "rez.txt"
    */

    // po defaultu nema FASTA datoteke
    std::string fastaFile = "";

    // default ime izlazne datoteke (ako se ne navede kao argument kom. linije)
    std::string outputFile = "results.txt";

    
    //prolazi kroz sve argumente komandne linije
       
    
    for (int i = 1; i < argc; i++)
    {
        // trenutni argument pretvara u string
        std::string arg = argv[i];

        
        // ako korisnik napise --file imeDatoteke sprema ime FASTA datoteke
        if (arg == "--file")
        {
            // argv[++i] uzima sljedeci argument
            fastaFile = argv[++i];
        }

        
        //ako korisnik napise --out imeRezultata sprema ime izlazne datoteke
        else if (arg == "--out")
        {
            outputFile = argv[++i];
        }
    }

    /*
        pokrece eksperimente nad Bamboo filterom

        ako postoji FASTA file:
        -> koristi stvarne DNA podatke

        ako ne postoji:
        -> koristi random generirane DNA sekvence
    */
    runExperiments(
        outputFile,
        fastaFile);

    // poruka nakon zavrsetka programa da su eksperimenti dovrseni, rezultati spremljeni u izlaznoj datoteci
    std::cout
        << "Experiments completed.\n";
    
    return 0;
}
