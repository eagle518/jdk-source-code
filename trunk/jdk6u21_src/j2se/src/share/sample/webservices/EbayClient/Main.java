/*
 * Main.java
 *
 * Created on June 29, 2006, 10:31 AM
 *
 * To change this template, choose Tools | Template Manager
 * and open the template in the editor.
 */

package ebay.client;
import ebay.apis.*;
import javax.xml.ws.*;

/**
 *
 * @author mode
 */
public class Main {
    
    private static final String devId = "D5X81QBY4E18QXIZ2PT3P1BGA9U731";

    private static final String appId = "SUNMICROSYF443CDQKFFGHG3L289T2";

    private static final String certId = "I8189L714K5$1SFJO75T1-B1H2KAKN";
    
    private static final String authToken = "AgAAAA**AQAAAA**aAAAAA**ocM5RA**nY+sHZ2PrBmdj6wVnY+sEZ2PrA2dj6wJnY+lDpmHpQidj6x9nY+seQ**YXYAAA**AAMAAA**3gg6xzYuOKqHrA+YXyfAxP/T8FWOkudIBSYZBKDtiAJFUUayagydwC9EeofVzScnH/ctqXNyzjN7MbUCva+PbjcwVSgEyPFBhK/0PtLWXPxIqJavJTAcXF81A+lLt4wzdt9TkBLv3tqsmaLmHJtFNiTWCRZYblR35bgmO0ZWZPhziwEWUkp24H3XIq8bsr6vcCVB3+pPzh4XhVM/oN81rxowcekESINbdqMhqHYbfa/21SV1uyYPBxwV2jd5+o+Vo77espSptOZcK9O2TQe+J3LADWqegQs229kmtQPcvHlhC/3ej4eAPvy3bMTzeGOJMw/bUyWSHkZpGSS4O7McaU89LFd4Mx7tRGDNCSNzfRxubHFoh01CYTnflm3DaZF+KcRPE1zvVGdpVLizPSJ/xiQEHErfkryKAQjNgwd0COUqFlOe7BLkf1YfU1gly3BPjVUWb2EglGw4Lv0XvLhRG+xMV0Lve8kF2TmMGbJlhS9osweO9RwgjN8afck7mrdodc+PeKJPusDnbV8zRmUbelBnZ6Z+TiCQh9gKjvCtjUQ7mBlkldl1KXIpRhaIcLK24H0VVAqMNaonIkSMrJy3yUPoAcLyyHMS0RP6hHWTVa/+4odt/jAVDFsX9UE/iD/j5HpQK8Pqw2kQbth6uBwDedJEBqQY/dJ/hEQsa/N7pTsRzwZPLcvCFFnqlOd4EOyx3CsPe1LCEAjo1KWtDxQDpCOzMDmC8WTGHlrrKXOlGbV0NzoKrGc8m5ZdWw8SZQl4";
    
    private static final String prodDevID = "D5X81QBY4E18QXIZ2PT3P1BGA9U731";

    private static final String prodAppID = "SUNMICROSYH2IQ3HO1U9U8T8FT8247";

    private static final String prodCertID = "A672S4143Y6$CP154489B-Y165819Y";

    private static final String prodAuthToken = "AgAAAA**AQAAAA**aAAAAA**m6I8RA**nY+sHZ2PrBmdj6wVnY+sEZ2PrA2dj6wJlYumDpCEqAudj6x9nY+seQ**/xUAAA**AAMAAA**ZGnZW07oWCuANb8OxUcbL7QzWVcBQ4P3zKBt3wcmikV4RVyBgIcXRcjpFbrjxbC5HF0O4djZBGft3+kFO7Q8la1R+cssnw2ovtdmKiogetza/N7ku7I1q8wKj/5buBtocmaG2IQycV96V80HtZqvaQ755xXN3eAl2o05O7gWvqAKbZeV6LO5jTlRbKgh13DRaYglNKetqLQqqcPRSH6FOa3yO5OPFxRVjoBimWOCiLXj0Fhj2XZPQvVeymifYrT9eibf/cf3mrcHAnR0BL15Ihd5l2hnX9av76q/8w92gp1a8nYTjPacVv7gxtLaQM7ozg7LKPoiaguoyxC3fLFfMu3DF+iJMu8MTWUJinrP/jt47Pku6Ap5HbnDR9lq1ahsne5Ev0qMJasxv57Zydrz6segEqIZTGqUSrcrifnR/KicH9OuLdWJFvEl3RhaXhuHiSR6CkA3eFkuhKl55p1UeRbLPvyIB36+QrTJfxJGeEu+qcCNGnljGi7hDIJfKz5ioL0HHIZ2XonEQikbULRzMT9x6BOTs0CuTrqlBvQUYyVk+fdcxymHcoqyj46HpCcEkVxnsUOABDixQW20IrgsmQJyXUZfZIvVsljuxjMuv4WQJCH8WP3+Nsnr1IXhIuleJ33P1Ordda+sRI80sFCohj1d8J/gU3EJ9MO8QJmjQIEdR9slPq5qEgNEzB74O25j6Ti0s1UBBfx/QvUVpcRG+yG9lEHaENNtdIeAd3umL8NiXeceaT4hQiMbzj3rfft/";
    
    private static final String baseURL = "https://api.ebay.com/wsapi?";
    
    private static final String localURL = "http://localhost:7070/Ebay"; 
    
    private static CustomSecurityHeaderType header;

    static {
        UserIdPasswordType userId = new UserIdPasswordType();
        userId.setAppId(prodAppID);
        userId.setDevId(prodDevID);
        userId.setAuthCert(prodCertID);
        
        header = new CustomSecurityHeaderType();
        header.setEBayAuthToken(prodAuthToken);
        header.setCredentials(userId);
    }
    
    /** Creates a new instance of Main */
    public Main() {
    }
    
    /**
     * @param args the command line arguments
     */
    public static void main(String[] args) {
        if(args.length <2) {
            System.out.println("Usage: java -jar EbayMustangClient" + 
                               " <endpoint to use> <item number>");
            System.exit(1);
        }
        
        ItemType item = getItem(args[0], args[1]);
   
        if (item == null) {
            System.out.println("Could not find item");
        }
        else {
            System.out.println("Found item: " + item.getTitle());
            if (item.getPictureDetails() != null) {
                System.out.println("Picture url is " + item.getPictureDetails().getGalleryURL());
            }
            if (item.getDescription() != null) {
                System.out.println("Description: " + item.getDescription());
            }
            if (item.getBestOfferDetails() != null) {
                System.out.println("Current bid: " + item.getBestOfferDetails().getBestOffer().getValue());
            }
            if (item.getBuyItNowPrice() != null) {
                System.out.println("Buy it now: " + item.getBuyItNowPrice().getValue());
            }
        }
    }
    
    public static ItemType getItem(String endpointToUse, String itemId) {
      boolean error = false;
      String endpointURL = "";
      EBayAPIInterfaceService svc = new EBayAPIInterfaceService();
      EBayAPIInterface port = svc.getEBayAPI();
      
        
        BindingProvider bp = (BindingProvider) port;
        if (endpointToUse.equalsIgnoreCase("ebay")) {
            endpointURL =  baseURL + "callname=GetItem&siteid=0&appid=" + 
                          prodAppID + "&version=455&Routing=new";
        }
        else if(endpointToUse.equalsIgnoreCase("local")) {
            endpointURL = localURL;
        }
        else {
            System.out.println("invalid endpoint");
            System.exit(1);
        }
        

        System.out.println("endpointURL is " + endpointURL);
        bp.getRequestContext().put(BindingProvider.ENDPOINT_ADDRESS_PROPERTY,
                                   endpointURL);
                
        

        GetItemRequestType itemRequest = new GetItemRequestType();
        itemRequest.setVersion("455");
        //itemRequest.setItemID("9707045425");

        itemRequest.setItemID(itemId);
        itemRequest.setErrorLanguage("en_US");
        GetItemResponseType itemResponse = null;
        try {
            itemResponse = port.getItem(itemRequest, header); 
        }catch (Exception e) {
            error = true;
        }
        if (error) {
            return null;
        }
        else 
            return itemResponse.getItem();
        
        
    }
}
