package com.titan.clients;

import javax.persistence.*;
import javax.ejb.*;
import java.io.*;

import com.titan.domain.*;
import java.util.List;
import java.util.Date;


public class InitializeDB
{
   public static void initialize(EntityManager manager)
   {
      List list = manager.createQuery("FROM Reservation res").getResultList();
      if (list.size() > 0) 
      {
        return;
      }
      

      Customer bill = new Customer();
      bill.setFirstName("Bill");
      bill.setLastName("Burke");
      bill.setHasGoodCredit(true);

      Customer sacha = new Customer();
      sacha.setFirstName("Sacha");
      sacha.setLastName("Labourey");
      sacha.setHasGoodCredit(false); // Sacha get's bad credit ;)

      Customer marc = new Customer();
      marc.setFirstName("Marc");
      marc.setLastName("Fleury");
      marc.setHasGoodCredit(true);

      Customer monica = new Customer();
      monica.setFirstName("Monica");
      monica.setLastName("Burke");
      monica.setHasGoodCredit(true);

      Customer gavin = new Customer();
      gavin.setFirstName("Gavin");
      gavin.setLastName("King");
      gavin.setHasGoodCredit(true);

      Customer rmh = new Customer();
      rmh.setFirstName("Richard");
      rmh.setLastName("Monson-Haefel");
      rmh.setHasGoodCredit(false);
      manager.persist(rmh);

      Address addr = new Address();
      addr.setStreet("123 Boston Road");
      addr.setCity("Billerica");
      addr.setState("MA");
      addr.setZip("02821");
      bill.setAddress(addr);
      Phone phone = new Phone();
      phone.setNumber("978-555-5555");
      bill.getPhoneNumbers().add(phone);
      phone = new Phone();
      phone.setNumber("617-555-5555");
      bill.getPhoneNumbers().add(phone);

      addr = new Address();
      addr.setStreet("Etwa Schweitzer Strasse");
      addr.setCity("Neuchatel");
      addr.setState("Switzerland");
      addr.setZip("07711");
      sacha.setAddress(addr);

      addr = new Address();
      addr.setStreet("JBoss Dr.");
      addr.setCity("Atlanta");
      addr.setState("GA");
      addr.setZip("96660");
      marc.setAddress(addr);

      addr = new Address();
      addr.setStreet("JBoss Dr.");
      addr.setCity("Atlanta");
      addr.setState("GA");
      addr.setZip("96660");
      gavin.setAddress(addr);

      addr = new Address();
      addr.setStreet("123 Boston Road");
      addr.setCity("Billerica");
      addr.setState("MA");
      addr.setZip("26660");
      monica.setAddress(addr);
      phone = new Phone();
      phone.setNumber("617-555-5555");
      monica.getPhoneNumbers().add(phone);

      CreditCompany capitalOne = new CreditCompany();
      capitalOne.setName("Capital One");
      addr = new Address();
      addr.setStreet("1 Federal Street");
      addr.setState("VA");
      addr.setCity("Richmond");
      addr.setZip("03032");
      capitalOne.setAddress(addr);
      manager.persist(capitalOne);

      CreditCompany mbna = new CreditCompany();
      mbna.setName("MBNA");
      addr = new Address();
      addr.setStreet("1 Federal Street");
      addr.setState("NC");
      addr.setCity("Charllote");
      addr.setZip("07732");
      mbna.setAddress(addr);
      manager.persist(mbna);

      CreditCard cc;
      cc = new CreditCard();
      cc.setExpirationDate(new Date());
      cc.setNumber("5324 9393 1010 2929");
      cc.setNameOnCard("Bill Burke");
      cc.setCreditCompany(capitalOne);
      bill.setCreditCard(cc);

      cc = new CreditCard();
      cc.setExpirationDate(new Date());
      cc.setNumber("3311 5000 1011 2333");
      cc.setNameOnCard("Sacha Labourey");
      cc.setCreditCompany(capitalOne);
      sacha.setCreditCard(cc);

      cc = new CreditCard();
      cc.setNumber("4310 5131 7711 2663");
      cc.setNameOnCard("Marc Fleury");
      cc.setCreditCompany(mbna);
      marc.setCreditCard(cc);

      cc = new CreditCard();
      cc.setNumber("4310 5144 7711 2663");
      cc.setNameOnCard("Monica Burke");
      cc.setCreditCompany(mbna);
      monica.setCreditCard(cc);

      cc = new CreditCard();
      cc.setNumber("5310 5144 7711 2663");
      cc.setNameOnCard("Gavin King");
      cc.setCreditCompany(mbna);
      gavin.setCreditCard(cc);

      manager.persist(bill);
      manager.persist(sacha);
      manager.persist(marc);
      manager.persist(monica);
      manager.persist(gavin);

      Ship queenMary = new Ship("Queen Mary", 40000.0);
      manager.persist(queenMary);
      Ship titanic = new Ship("Titanic", 100000.0);
      manager.persist(titanic);
      
      // Create cabins
      Cabin cabin1 = new Cabin();
      cabin1.setDeckLevel(1);
      cabin1.setShip(queenMary);
      cabin1.setBedCount(1);
      cabin1.setName("Queen Cabin 1");
      manager.persist(cabin1);

      Cabin cabin2 = new Cabin();
      cabin2.setDeckLevel(1);
      cabin2.setShip(queenMary);
      cabin2.setBedCount(1);
      cabin2.setName("Queen Cabin 2");
      manager.persist(cabin2);
         
      Cabin cabin3 = new Cabin();
      cabin3.setDeckLevel(1);
      cabin3.setShip(titanic);
      cabin3.setBedCount(2);
      cabin3.setName("Titanic Cabin 1");
      manager.persist(cabin3);
         
      Cabin cabin4 = new Cabin();
      cabin4.setDeckLevel(1);
      cabin4.setShip(titanic);
      cabin4.setBedCount(2);
      cabin4.setName("Titanic Cabin 2");
      manager.persist(cabin4);
         
      Cabin cabin5 = new Cabin();
      cabin5.setDeckLevel(1);
      cabin5.setShip(titanic);
      cabin5.setBedCount(2);
      cabin5.setName("Titanic Cabin 3");
      manager.persist(cabin5);

      // Create cruise
      Cruise alaskan = new Cruise("Alaskan Cruise", queenMary);
      manager.persist(alaskan);
      Cruise atlantic = new Cruise("Atlantic Cruise", titanic);
      manager.persist(atlantic);

      Reservation alaskanReservation = new Reservation();
      alaskanReservation.setCruise(alaskan);
      alaskanReservation.getCabins().add(cabin1);
      alaskanReservation.getCabins().add(cabin2);
      alaskanReservation.getCustomers().add(bill);
      alaskanReservation.getCustomers().add(sacha);
      manager.persist(alaskanReservation);
      
      Reservation atlanticReservation = new Reservation();
      atlanticReservation.setCruise(atlantic);
      atlanticReservation.getCabins().add(cabin3);
      atlanticReservation.getCustomers().add(marc);
      atlanticReservation.setAmountPaid(42.00);
      manager.persist(atlanticReservation);

      Reservation atlantic2 = new Reservation();
      atlantic2.setCruise(atlantic);
      atlantic2.getCabins().add(cabin5);
      atlantic2.getCustomers().add(bill);
      atlantic2.setAmountPaid(500.0);
      manager.persist(atlantic2);
   }
}

