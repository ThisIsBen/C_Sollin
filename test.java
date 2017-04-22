
public class HW1 {

	public static void main(String[] args) {
		// TODO Auto-generated method stub
		String input = args[0];//GET INPUT
		String tag="<tag>";
		int lenOfTag=tag.length();//GET LEN of <tag>
		//System.out.println(input);
		int tagBegin= input.indexOf("<tag>") ;

		int tagEnd= input.indexOf("</tag>"); 
		
		String target=input.substring(tagBegin+lenOfTag,tagEnd);//get value in <tag></tag>
		//System.out.println(target);//get target!
		
		// Can get target successfully
		
		int lenOfTarget=target.length();//get len of target attribute

		
		int targetBegin= input.indexOf(target,tagEnd) ;//the first target's index following the <tag> //20
		int targetEnd=0;
		String attribute="";
		
		//get all the target attributes throughout the input XML.
		while(targetBegin!=-1)//if target is not found
		{	
		
		
				targetEnd= input.indexOf(">",targetBegin); //get the index of the ">" which follows the target attribute 
		
				attribute=input.substring(targetBegin+lenOfTarget,targetEnd);//get target attribute
		
				
				attribute=attribute.trim();//trim " "
				
				
				//get rid of the "/" at the end of the attribute value.
				if( attribute.length()>0 && attribute.charAt(attribute.length()-1)=='/')
				{
					attribute=attribute.substring(0,attribute.length()-1);
				}
					
				
				System.out.println(attribute);//print out target attribute!
				
				targetBegin= input.indexOf(target,targetEnd) ;//the get next target's index.
		}
		
		
			
	}

}
