#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct tipo_chave{
	char artista[31];
	int ano;
} TChave;

typedef struct tipo_album{
	char nome[51];
	int nFaixas, tempo;
} TAlbum;

typedef struct node{
	int nch, folha;
	TChave *chave;
	TAlbum *album;
	struct node **filho, *prev, *next;
} BMais;

BMais *aloca(int t){
	BMais *novo = (BMais*)malloc(sizeof(BMais));
	novo->chave = (TChave*)malloc(sizeof(TChave)*(2*t-1));
	novo->album = (TAlbum*)malloc(sizeof(TAlbum)*(2*t-1));
	novo->nch = 0;
	novo->folha = 1;
	novo->prev = NULL;
	novo->next = NULL;
	novo->filho = (BMais**)malloc(sizeof(BMais*)*(2*t));
	int i;
	for(i=0; i<2*t; i++) novo->filho[i] = NULL;
	return novo;
}

void libera(BMais *a){
	if(a){
		if(!a->folha){
			int i;
			for(i=0; i<a->nch; i++) libera(a->filho[i]);
		}
		free(a->chave);
		free(a->album);
		free(a->filho);
		free(a);
	}
}

int comparaChave(TChave a, TChave b){
	if(strcmp(a.artista, b.artista) < 0) return -1;
	if(strcmp(a.artista, b.artista) > 0) return 1;
	if(a.ano < b.ano) return -1;
	if(a.ano > b.ano) return 1;
	return 0;
}

BMais* busca(BMais *a, TChave chave, int t){
	if(!a) return a;
	int i=0;
	while((i < a->nch) && (comparaChave(a->chave[i], chave) <= 0))i++;
	if((a->folha) && (i-1 < a->nch) && (comparaChave(chave, a->chave[i-1]) == 0))
		return a;
	return busca(a->filho[i], chave, t);
}

BMais* divisao(BMais *pai, BMais *dividindo, int t, int indFilhoNovo){
	BMais *filhoNovo = aloca(t);
	filhoNovo->folha = dividindo->folha;
	int j;
	for(j=pai->nch; j>=indFilhoNovo; j--){
		pai->filho[j+1] = pai->filho[j];
		pai->chave[j] = pai->chave[j-1];
	}
	pai->filho[indFilhoNovo] = filhoNovo;
	pai->chave[indFilhoNovo-1] = dividindo->chave[t-1];
	if(dividindo->folha){
		filhoNovo->nch = t;
		for(j=0; j<t; j++){
			filhoNovo->chave[j] = dividindo->chave[j+t-1];
			filhoNovo->album[j] = dividindo->album[j+t-1];
		}
		filhoNovo->next = dividindo->next;
		filhoNovo->prev = dividindo;
		dividindo->next = filhoNovo;
	}
	else{
		filhoNovo->nch = t-1;
		for(j=0; j<t-1; j++)
			filhoNovo->chave[j] = dividindo->chave[j+t];
		for(j=0; j<t+1; j++){
			filhoNovo->filho[j] = dividindo->filho[j+t];
			dividindo->filho[j+t] = NULL;
		}
	}
	dividindo->nch = t-1;
	pai->nch++;
	return pai;
}

BMais* insereNoImcompleto(BMais *a, TChave chave, TAlbum album, int t){
	int i = a->nch - 1;
	if(a->folha){
		while((i>=0) && (comparaChave(chave, a->chave[i]) < 0)){
			a->chave[i+1] = a->chave[i];
			a->album[i+1] = a->album[i];
			i--;
		}
		if(comparaChave(chave, a->chave[i]) == 0) return a;
		a->chave[i+1] = chave;
		a->album[i+1] = album;
		a->nch++;
		return a;
	}
	while((i>=0) && (comparaChave(chave, a->chave[i]) < 0))i--;
	i++;
	if(a->filho[i]->nch == 2*t-1){
		a = divisao(a, a->filho[i], t, i+1);
		if(comparaChave(chave, a->chave[i]) > 0)i++;
	}
	a->filho[i] = insereNoImcompleto(a->filho[i], chave, album, t);
	return a;
}

BMais* insere(BMais *a, TChave chave, TAlbum album, int t){
	if(!a){
		a = aloca(t);
		a->chave[0] = chave;
		a->album[0] = album;
		a->nch++;
		return a;
	}
	if(a->nch == 2*t-1){
		BMais *novaRaiz = aloca(t);
		novaRaiz->folha = 0;
		novaRaiz->filho[0] = a;
		novaRaiz = divisao(novaRaiz, a, t, 1);
		novaRaiz = insereNoImcompleto(novaRaiz, chave, album, t);
		return novaRaiz;
	}
	a = insereNoImcompleto(a, chave, album, t);
	return a;
}

BMais* remover(BMais *a, TChave chave, int t){
	if(!a) return a;
	int i;
	for(i=0; i < a->nch && comparaChave(a->chave[i], chave) < 0; i++);
	if(i < a->nch && comparaChave(chave, a->chave[i]) == 0){
		if(a->folha){
			int j;
			for(j=i; j < a->nch-1; j++){
				a->chave[j] = a->chave[j+1];
				a->album[j] = a->album[j+1];
			}
			a->nch--;
			return a;
		}
		i++;
	}
	BMais *y = a->filho[i], *z = NULL;
	if(y->nch == t-1){
		if((i < a->nch) && (a->filho[i+1]->nch >= t)){
			z = a->filho[i+1];
			if(!y->folha){
				y->chave[t-1] = a->chave[i];
				a->chave[i] = z->chave[0];
			}
			else{
				y->chave[t-1] = z->chave[0];
				y->album[t-1] = z->album[0];
				a->chave[i] = z->chave[1];
			}
			y->nch++;
			int j;
			for(j=0; j < z->nch-1; j++){
				z->chave[j] = z->chave[j+1];
				if(z->folha) z->album[j] = z->album[j+1];
			}
			y->filho[y->nch] = z->filho[0];
			for(j=0; j < z->nch; j++)
				z->filho[j] = z->filho[j+1];
			z->nch--;
			a->filho[i] = remover(a->filho[i], chave, t);
			return a;
		}
		if((i > 0) && (!z) && (a->filho[i-1]->nch >= t)){
			z = a->filho[i-1];
			int j;
			for(j = y->nch; j > 0; j--){
				y->chave[j] = y->chave[j-1];
				if(y->folha)y->album[j] = y->album[j-1];
			}
			for(j = y->nch+1; j > 0; j--)
				y->filho[j] = y->filho[j-1];
			if(!y->folha){
				y->chave[0] = a->chave[i-1];
				a->chave[i-1] = z->chave[z->nch-1];
				y->filho[0] = z->filho[z->nch];
			}
			else{
				y->chave[0] = z->chave[z->nch-1];
				y->album[0] = z->album[z->nch-1];
				a->chave[i-1] = z->chave[z->nch-1];
			}
			y->nch++;
			z->nch--;
			a->filho[i] = remover(a->filho[i], chave, t);
			return a;
		}
		if(!z){
			if(i < a->nch && a->filho[i+1]->nch == t-1){
				z = a->filho[i+1];
				if(!y->folha){
					y->chave[t-1] = a->chave[i];
					y->nch++;
				}
				int nchavesAtual = y->nch;
				int j;
				for(j = 0; j < t-1; j++){
					y->chave[j+nchavesAtual] = z->chave[j];
					if(y->folha)y->album[j+nchavesAtual] = z->album[j];
					y->nch++;
				}
				if(!y->folha){
					for(j = 0; j < t; j++)
						y->filho[j+t] = z->filho[j];
				}
				else{
					y->next = z->next;
					z->next->prev = y;
				}
				for(j = i; j < a->nch-1; j++){
					a->chave[j] = a->chave[j+1];
					a->filho[j+1] = a->filho[j+2];
				}
				a->nch--;
				a = remover(a, chave, t);
				return a;
			}
			if((i > 0) && (a->filho[i-1]->nch == t-1)){
				z = a->filho[i-1];
				if(!z->folha){
					if(i == a->nch)
						z->chave[t-1] = a->chave[i-1];
					else
						z->chave[t-1] = a->chave[i];
					z->nch++;
				}
				int nchavesAtual = z->nch;
				int j;
				for(j = 0; j < t-1; j++){
					z->chave[j+nchavesAtual] = y->chave[j];
					if(z->folha)z->album[j+nchavesAtual] = y->album[j];
					z->nch++;
				}
				if(!z->folha){
					for(j = 0; j < t; j++)
						z->filho[j+t] = y->filho[j];
				}
				else{
					z->next = y->next;
				}
				a->nch--;
				a->filho[i-1] = z;
				a = remover(a, chave, t);
				return a;
			}
		}
	}
	a->filho[i] = remover(a->filho[i], chave, t);
	return a;
}

BMais* retira(BMais *a, TChave chave, int t){
	if(!a || !busca(a, chave, t))return a;
	return remover(a, chave, t);
}

BMais* leArquivo(int t){
	FILE *pt = fopen("arq_entrada_trab.txt", "r");
	char linha[256], *token;
	BMais *a = NULL;
	TAlbum alb;
	TChave ch;
	while(fscanf(pt, " %[^\n]s", linha) != EOF){
		printf("Linha lida: %s\nDivisao:\n", linha);
		token = strtok(linha, "/");
		strcpy(ch.artista, token);
		token = strtok(NULL, "/");
		ch.ano = atoi(token);
		token = strtok(NULL, "/");
		alb.nFaixas = atoi(token);
		token = strtok(NULL, "/");
		alb.tempo = atoi(token);
		token = strtok(NULL, "/");
		strcpy(alb.nome, token);
		
		a = insere(a, ch, alb, t);
	}
	fclose(pt);
	return a;
}

TChave recebeChave(){
	TChave novaChave;
	printf("Digite os dados da chave:\n");
	scanf(" %[^\n]s", novaChave.artista);
	scanf(" %d", &novaChave.ano);
	return novaChave;
}

TAlbum recebeAlbum(){
	TAlbum novoAlbum;
	printf("Digite os dados do album:\n");
	scanf(" %d %d", &novoAlbum.nFaixas, &novoAlbum.tempo);
	scanf(" %[^\n]s", novoAlbum.nome);
	return novoAlbum;
}

BMais* novaChave(BMais *a, int t){
	TChave chave = recebeChave();
	TAlbum album = recebeAlbum();
	return insere(a, chave, album, t);
}

void alteraChave(BMais *a, TChave chave, int t){
	BMais *res = busca(a, chave, t);
	if(!res){
		printf("Essa chave nao existe\n");
		return;
	}
	int i;
	for(i = 0; i < res->nch; i++){
		if(comparaChave(chave, res->chave[i]) == 0){
			printf("Digite as alteracoes:\n");
			scanf(" %d %d %[^\n]s", &res->album[i].nFaixas, &res->album[i].tempo, res->album[i].nome);
			return;
		}
	}
}

void buscaObrasArtista(BMais* a, char *artista){
    int i = 0;
    while ((i < a->nch) && (strcmp(a->chave[i].artista, artista) < 0)){
        i++;
    }
    if (a->folha){
        BMais* aux = a;
        while(aux){
            for(i=0; i<aux->nch; i++)
                if (strcmp(aux->chave[i].artista, artista) == 0)
                    printf("%s / %d / %d / %d / %s\n", aux->chave[i].artista, aux->chave[i].ano, aux->album[i].nFaixas, aux->album[i].tempo, aux->album[i].nome);

            aux = aux->next;
        }
        printf("\n");
        return;
    }
    buscaObrasArtista(a->filho[i], artista);
}

BMais* removerArtista(BMais *a, char *artista, int t){
    int i = 0;
    while ((i < a->nch) && (strcmp(a->chave[i].artista, artista) < 0)){
        i++;
    }
    if (a->folha){
        int j;
        for(j=0; j<a->nch; j++)
            if (strcmp(a->chave[j].artista, artista) == 0){
                a = retira(a, a->chave[j], t);
                removerArtista(a, artista, t);
            }
        printf("\n");
        return a;
    }
    a->filho[i] = removerArtista(a->filho[i], artista, t);
    return a;
}

void imprimeLista(BMais *a){
    if(!a){
        printf("Playlist vazia\n");
        return;
    }
    BMais *aux = a;
    while(aux->filho[0])aux = aux->filho[0];
    while(aux){
        int i;
        for(i = 0; i < aux->nch; i++)
            printf("%s / %d\n", aux->chave[i].artista, aux->chave[i].ano);
        aux = aux->next;
    }
}

void buscaAlbum(BMais *a, TChave chave, int t){
    BMais *res;
    res = busca(a, chave, t);
    if(!res){
        printf("Album nao encontrado\n");
        return;
    }
    int i;
    for(i = 0; i < res->nch; i++)
        if(comparaChave(chave, res->chave[i]) == 0){
            printf("%s / %d / %d / %d / %s\n", res->chave[i].artista, res->chave[i].ano, res->album[i].nFaixas, res->album[i].tempo, res->album[i].nome);
            return;
        }
}

int main(){
    BMais *playlist = leArquivo(2);
    int cmd;
    while(1){
        printf("-----------------------------\n"
               "Menu:\n\n"
               "(0) Ver playlist\n"
               "(1) Adicionar novo album\n"
               "(2) Remover album\n"
               "(3) Editar album\n"
               "(4) Buscar album\n"
               "(5) Buscar artista\n"
               "(6) Remover artista\n"
               "(7) Sair\n\n"
               "-----------------------------\n");
        scanf("%d", &cmd);
        if(cmd == 0){
            imprimeLista(playlist);
        }
        else if(cmd == 1){
            playlist = novaChave(playlist, 2);
        }
        else if(cmd == 2){
            playlist = retira(playlist, recebeChave(), 2);
        }
        else if(cmd == 3){
            alteraChave(playlist, recebeChave(), 2);
        }
        else if(cmd == 4){
            buscaAlbum(playlist, recebeChave(), 2);
        }
        else if(cmd == 5){
            char artista[31];
            printf("Digite o nome do artista:\n");
            scanf(" %[^\n]s", artista);
            buscaObrasArtista(playlist, artista);
        }
        else if(cmd == 6){
            char artista[31];
            printf("Digite o nome do artista:\n");
            scanf(" %[^\n]s", artista);
            playlist = removerArtista(playlist, artista, 2);
        }
        else if(cmd == 7){
            libera(playlist);
            break;
        }
    }
    return 0;
}



