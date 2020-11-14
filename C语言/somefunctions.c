#include<stdio.h>
char* StrCat(char* s1, const char* s2){
	int i, j;
	for(i=0; *(s1+i) != 0; i++);
	for(j=0; *(s2+j)!=0; j++) *(s1+i++) = *(s2+j);
	*(s1+i) = 0;
	return s1;
} 
int StrCmp(const char* s1, const char* s2){
	int i;
	for(i=0; *(s1+i) == *(s2+i) && (*(s1+i) != 0 || *(s2+i) != 0);i++);
	return *(s1+i) - *(s2+i);
}
char* Copy(char* s1, const char* s2){
	int i;
	for(i=0; *(s2+i)!=0; i++) *(s1+i) = *(s2+i);
	*(s1+i) = 0;
	return s1;
}
char* StrUpr(char* s){
	int i;
	for(i=0; *(s+i); i++){
		if(*(s+i) >= 'a' && *(s+i) <= 'z') *(s+i) -= 32;
	}
	return s;
}
char* StrLwr(char *s){
	int i;
	for(i=0; *(s+i); i++){
		if(*(s+i) >= 'A' && *(s+i) <= 'Z') *(s+i) += 32;
	}
	return s;
}
int charAt(const char* s, char c){
	int i, pos = -1;
	for(i=0; *(s+i); i++){
		if(*(s+i) == c) {pos = i;break;
		}
	}
	return pos;
}
int isNumberChar(char c){
	if(c >= '0' && c <= '9') return 1;
	return 0;
}
//求字符串中的所有整数(不是所有的数字) 
int numbers(int nums[], const char* s){
	int i=0, total = 0, isNum = 0, value = 0, j;
	char c;
	while(c=*(s+i)){
		if(!isNumberChar(c)) {
			isNum = 0;
		}
		else if(isNum == 0){
				isNum = 1; 
				value = c - '0';
				j = i+1; 
				while(*(s+j) && isNumberChar(*(s+j))){
					value = value*10 + *(s+j) - '0';
					j++;
				} 
				nums[total] = value;
				total++;
				i = j-1;
		}
		i++;
	}
	return total;
}

//模式匹配，BF暴力算法 
int bf(const char*s, const char* t){
	int i=0, j=0, k=0, tlen = length(t);
	for(k=0; s[k+tlen]!=0; k++){
		i=k;
		for(j=0;j<tlen; j++){
			if(s[i++] != t[j])break;
		}
		if(j == tlen) break;		
	}
	if(k+tlen <= length(s))
		return k;
	return -1;
}

int main(){
	return 0;
}
